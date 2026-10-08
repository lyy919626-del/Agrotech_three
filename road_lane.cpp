#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

// 输入视频路径
const string VIDEO_PATH = "road_video.mp4";
// 偏移报警阈值：像素，大于该值判定偏离车道
const int OFFSET_THRESHOLD = 30;

// -------------------------------------------------------------
// 1. Canny 边缘检测：灰度化 -> 高斯模糊去噪 -> Canny 提取边缘
// -------------------------------------------------------------
Mat canny_edge(Mat img)
{
    Mat gray, blur_img, edges;
    cvtColor(img, gray, COLOR_BGR2GRAY);          // 转灰度，减少计算量
    GaussianBlur(gray, blur_img, Size(5, 5), 0);  // 高斯模糊去噪，避免误检
    Canny(blur_img, edges, 50, 150);              // 双阈值边缘检测
    return edges;
}

// -------------------------------------------------------------
// 2. ROI 掩码：用梯形只保留路面车道区域，去掉天空/车外背景
// -------------------------------------------------------------
Mat roi_mask(Mat edges)
{
    int height = edges.rows;
    int width = edges.cols;
    // 梯形顶点（车载视角路面：底部宽、顶部窄，汇聚到远处）
    vector<Point> points =
    {
        Point(width * 0.10, height),          // 左下
        Point(width * 0.45, height * 0.60),   // 左上
        Point(width * 0.55, height * 0.60),   // 右上
        Point(width * 0.90, height)           // 右下
    };
    // 全黑掩码 + 把梯形区域填成白色
    Mat mask = Mat::zeros(edges.size(), edges.type());
    fillConvexPoly(mask, points, Scalar(255));
    // 与运算：只保留梯形内的边缘
    Mat roi;
    bitwise_and(edges, mask, roi);
    return roi;
}

// -------------------------------------------------------------
// 3. 筛选左右车道线：按斜率区分，再最小二乘拟合出平均直线
// -------------------------------------------------------------
vector<Vec4i> average_lines(Mat img, vector<Vec4i> lines)
{
    vector<Vec4i> result;
    vector<Point2f> left_pts, right_pts;   // 收集左右车道线的点集
    if (lines.empty()) return result;
    for (size_t i = 0; i < lines.size(); i++)
    {
        int x1 = lines[i][0], y1 = lines[i][1];
        int x2 = lines[i][2], y2 = lines[i][3];
        if (x2 - x1 == 0) continue;   // 排除竖直线
        double slope = (double)(y2 - y1) / (x2 - x1);
        if (abs(slope) < 0.5) continue;  // 排除接近水平的干扰线
        // 斜率为负 -> 左车道线；斜率为正 -> 右车道线
        if (slope < 0)
        {
            left_pts.push_back(Point2f(x1, y1));
            left_pts.push_back(Point2f(x2, y2));
        }
        else
        {
            right_pts.push_back(Point2f(x1, y1));
            right_pts.push_back(Point2f(x2, y2));
        }
    }
    int height = img.rows;
    int y_bottom = height;             // 车道线下端点（画面底部）
    int y_top = (int)(height * 0.65);  // 车道线上端点（远处）
    // 最小二乘拟合左车道（fitLine 返回方向向量(vx,vy)和线上一点(x0,y0)）
    if (!left_pts.empty())
    {
        Vec4f l;
        fitLine(left_pts, l, DIST_L2, 0, 0.01, 0.01);
        int x1 = (int)(l[2] + (y_bottom - l[3]) * l[0] / l[1]);
        int x2 = (int)(l[2] + (y_top - l[3]) * l[0] / l[1]);
        result.push_back(Vec4i(x1, y_bottom, x2, y_top));
    }
    // 最小二乘拟合右车道
    if (!right_pts.empty())
    {
        Vec4f r;
        fitLine(right_pts, r, DIST_L2, 0, 0.01, 0.01);
        int x1 = (int)(r[2] + (y_bottom - r[3]) * r[0] / r[1]);
        int x2 = (int)(r[2] + (y_top - r[3]) * r[0] / r[1]);
        result.push_back(Vec4i(x1, y_bottom, x2, y_top));
    }
    return result;
}

// --------
// 4. 绘制车道线
//    画面中两条粗线的意义：
//      红线(i==0) = 拟合出来的左车道边线，对应现实道路左侧的标线
//      绿线(i==1) = 拟合出来的右车道边线，对应现实道路右侧的标线
//    两条线之间的范围就是程序识别出的当前可行驶车道。
// -------------------
void draw_lanes(Mat &img, vector<Vec4i> lanes)
{
    for (size_t i = 0; i < lanes.size(); i++)
    {
        Scalar color;
        // i==0：红色左车道线；i==1：绿色右车道线
        if (i == 0)
        {
            color = Scalar(0, 0, 255);   // Red 左车道
        }
        else
        {
            color = Scalar(0, 255, 0);   // Green 右车道
        }

        line(img, Point(lanes[i][0], lanes[i][1]),
            Point(lanes[i][2], lanes[i][3]), color, 8);
    }
}

// -------------------------------------------------------------
// 主函数
// -------------------------------------------------------------
int main()
{
    // 打开视频（优先加载同目录 road_video.mp4，失败则用摄像头）
    VideoCapture cap;
    cap.open(VIDEO_PATH);
    if (!cap.isOpened())
    {
        cout << "无法打开视频/摄像头！请确认 road_video.mp4 存在，或修改 VIDEO_PATH。" << endl;
        return -1;
    }
    Mat frame;
    while (true)
    {
        cap >> frame;
        if (frame.empty()) break;   // 视频播放结束
        resize(frame, frame, Size(800, 450));   // 统一尺寸便于显示
        // 边缘检测 -> 提取路面ROI -> 霍夫检测直线
        Mat edges = canny_edge(frame);
        Mat roi = roi_mask(edges);
        vector<Vec4i> lines;
        HoughLinesP(roi, lines,
            1,            // 距离分辨率 1 像素
            CV_PI / 180,  // 角度分辨率 1 度
            50,           // 累加器阈值（越大检测越严格）
            50,           // 最小线段长度
            10);          // 线段最大间隙
        // 4) 拟合左右车道线并绘制
        vector<Vec4i> lanes = average_lines(frame, lines);
        draw_lanes(frame, lanes);

        //  车道偏移判断与屏幕文字打印 
        // 判断原理：对比“车辆中心(画面水平中点)”与“车道中心线”的横向位置
        if (lanes.size() == 2)  // 必须同时检测到左、右两条车道线才计算偏移
        {
            // lanes[0] 红线(左车道)：lanes[0][0] 是左车道底部X坐标
            int x_left_bottom = lanes[0][0];
            // lanes[1] 绿线(右车道)：lanes[1][0] 是右车道底部X坐标
            int x_right_bottom = lanes[1][0];

            // 车道中心线x（左右车道线底部x坐标的中点）
            double lane_center_x = (x_left_bottom + x_right_bottom) / 2.0;
            // 车辆中心：取画面水平中点
            double car_center_x = frame.cols / 2.0;

            // offset = 车中心 − 车道中心
            double offset = car_center_x - lane_center_x;

            string statusText;
            Scalar textColor;
            if (fabs(offset) < OFFSET_THRESHOLD)
            {
                statusText = "Center";
                textColor = Scalar(0, 255, 0);   // 绿色
            }
            else if (offset > 0)
            {
                // 车中心在车道中心线右边 -> 偏右
                statusText = "Right";
                textColor = Scalar(0, 0, 255);   // 红色，醒目提示
            }
            else
            {
                // 车中心在车道中心线左边 -> 偏左
                statusText = "Left";
                textColor = Scalar(0, 0, 255);   // 红色，醒目提示
            }
            // 在画面左上角打印大号文字（偏左/偏右用红色，居中用绿色）
            putText(frame, statusText, Point(40, 70),
                FONT_HERSHEY_SIMPLEX, 2.0, textColor, 4);
        }
        else
        {
            // 没有同时识别出两条车道线，打印提示
            putText(frame, "None", Point(40, 70),
                FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 3);
        }
        // =========================================================

        imshow("车道线检测", frame);   // 显示结果
        // imshow("ROI边缘", roi);     // 想看中间过程可取消注释
        char key = waitKey(30);
        if (key == 27 || key == 'q' || key == 'Q') break;  // ESC 或 Q 退出
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
