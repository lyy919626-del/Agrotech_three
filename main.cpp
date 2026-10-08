#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int main() 
{
    // ================= 1. 读取摄像头实时画面 =================
    VideoCapture cap(0); 
    if (!cap.isOpened()) 
    {
        cout << "无法打开摄像头或视频" << endl;
        return -1;
    }

    Mat frame, gray, binary, result;
    
    while (true) 
    {
        // 读取一帧画面
        cap >> frame;
        if (frame.empty()) 
        {
            break;
        }

        // 基础4：显示原始画面  复制一个原始画面
        frame.copyTo(result);

        /* 图像预处理与二值化 */ 
        //基础2： 转换为灰度图
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // 进阶1：必要的去噪 高斯
        // 使用5x5的卷积核，消除图像中的高频噪点
        GaussianBlur(gray, gray, Size(5, 5), 0);

        // 基础3：不得只使用单一固定阈值 自适应二值化
        // 使用 THRESH_OTSU，根据现场光照变化选择或调整二值化阈值
        threshold(gray, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);

        // 进阶1：形态学处理 使用开运算
        Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
        morphologyEx(binary, binary, MORPH_OPEN, kernel);

        /*轮廓检测与形状识别*/ 
        vector<vector<Point>> contours; // 存储所有轮廓
        vector<Vec4i> hierarchy;        // 轮廓的层级信息

        // RETR_EXTERNAL只找最外层，避免嵌套干扰
        findContours(binary, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);


    //     for (size_t i = 0; i < contours.size(); i++) 
    //     {
    //         // 过滤掉太小的轮廓、噪点
    //         double area = contourArea(contours[i]);
    //         if (area < 100) continue; // 注意：如果你要检测极小形状，这里可能要调到 50 甚至更小

    //         // 多边形逼近（用于形状分类）
    //         double peri = arcLength(contours[i], true);
    //         vector<Point> approx;
    //         approxPolyDP(contours[i], approx, 0.02 * peri, true); // 适当调小精度以适应小物体 一开始上网找的进度，这个是后面调的

    //         // 获取最小外接矩形 
    //         RotatedRect minRect = cv::minAreaRect(contours[i]);
    //         Rect boundingRect = minRect.boundingRect(); // 仅用于计算文字摆放位置 不然乱飞 后面调整的

    //         string shapeName = "Unknown";
    //         Scalar color;

    //         // 判断形状：根据顶点数量
    //         if (approx.size() == 3) 
    //         {
    //             shapeName = "Triangle";
    //             color = Scalar(0, 0, 255); // 红色
    //         } 
    //         else if (approx.size() == 4) 
    //         {
    //             float aspectRatio = (float)boundingRect.width / (float)boundingRect.height;
    //             if (aspectRatio >= 0.85 && aspectRatio <= 1.15) 
    //             {
    //                 shapeName = "Square";
    //                 color = Scalar(0, 255, 0); // 绿色
    //             } else {
    //                 shapeName = "Rectangle";
    //                 color = Scalar(255, 0, 0); // 蓝色
    //             }
    //         } 
    //         else if (approx.size() >= 5) 
    //         { // 稍微放宽圆的判定条件
    //             Point2f center;
    //             float radius;
    //             minEnclosingCircle(contours[i], center, radius);
    //             float circleArea = 3.14159 * radius * radius;
    //             float ratio = area / circleArea;
    //             if (ratio > 0.7)
    //             {
    //                 shapeName = "Circle";
    //                 color = Scalar(0, 255, 255); // 黄色
    //             }
    //     }

    //     // 绘制结果 
    //     if (shapeName != "Unknown") {
    //         //  绘制轮廓
    //         drawContours(result, contours, (int)i, color, 1);

    //         //  绘制最小外接矩形（倾斜的框）
    //         Point2f rect_points[4];
    //         minRect.points(rect_points); // 获取四个顶点
    //         for (int j = 0; j < 4; j++) {
    //             // 连接相邻的两个顶点
    //             line(result, rect_points[j], rect_points[(j + 1) % 4], color, 1);
    //         }

    //         // 绘制类别名称放在水平外接框的左上角
    //         putText(result, shapeName, Point(boundingRect.x, boundingRect.y - 5),
    //                 FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    //     }
    // }







        for (size_t i = 0; i < contours.size(); i++) 
        {
            // 过滤掉太小的轮廓、噪点
            double area = contourArea(contours[i]);  //计算面积
            if (area < 1000) continue; // 面积小于1000像素的忽略

            //  进阶2：识别形状 多边形逼近
            // 计算轮廓的周长
            double peri = arcLength(contours[i], true);   //选择闭合的为所需要的轮廓
            // 用多边形逼近轮廓，0.04*peri 是经验值，表示逼近精度，这个精度我上网查的
            vector<Point> approx;  //存储多边形顶点
            approxPolyDP(contours[i], approx, 0.04 * peri, true);

            //  获取轮廓的边界框 用于绘制
            Rect boundingRect = cv::boundingRect(contours[i]);
            string shapeName = "Unknown";
            Scalar color;

            // 根据顶点数量判断形状
            if (approx.size() == 3) 
            {
                // 3个顶点：三角形
                shapeName = "Triangle";
                color = Scalar(0, 0, 255); // 红色
            } 
            else if (approx.size() == 4) 
            {
                // 4个顶点：矩形或正方形  计算边界框的宽高比
                
                float aspectRatio = (float)boundingRect.width / (float)boundingRect.height;
                // 宽高比接近1，认为是正方形 允许一定误差，比如0.85到1.15之间？这个范围调一下
                if (aspectRatio >= 0.85 && aspectRatio <= 1.15) 
                {
                    shapeName = "Square";
                    color = Scalar(0, 255, 0); // 绿色
                } 
                else 
                {
                    shapeName = "Rectangle";
                    color = Scalar(255, 0, 0); // 蓝色
                }
            } 
            else if (approx.size() > 6) {
                // 顶点数大于4，可能是圆形 ，不过可以试一下6，但是这样就中间有空出来的没检测
                // 通过判断轮廓面积与最小外接圆面积的比值来确认 圆形的比值接近1 ，这个借鉴了csdn上的 之前也试用过
                Point2f center;
                float radius;
                minEnclosingCircle(contours[i], center, radius);
                float circleArea = 3.14159 * radius * radius;  //计算圆的面积
                float ratio = area / circleArea;   //占比
                
                if (ratio > 0.7) { // 如果轮廓占据外接圆的70%以上，认为是圆 调一下
                    shapeName = "Circle";
                    color = Scalar(0, 255, 255); // 黄色
                }
            }

            //  进阶3 绘制结果 
            if (shapeName != "Unknown")  //有轮廓
            {
                // 绘制轮廓 在原图上画
                drawContours(result, contours, (int)i, color, 2);
                
                // 绘制识别框
                rectangle(result, boundingRect, color, 2);
                
                // 绘制类别名称
                // 将类别名称的文字放在识别框的左上方
                putText(result, shapeName, Point(boundingRect.x, boundingRect.y - 10),
                        FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
            }
        }

        // 基础4 ：实时显示画面
        imshow("1. Original Video原始", frame);  // 原始画面
        imshow("2. Binary Image二值化", binary);   // 二值化结果
        imshow("3. Final Result最终", result);   // 最终识别结果

        // 按下 ESC 键（键值 27）退出循环
        int key = waitKey(30);
        if (key == 27) 
        {
            break;
        }
    }

    // 释放摄像头
    cap.release();
    //destroyAllWindows();

    return 0;
}