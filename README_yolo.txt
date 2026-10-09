
一、任务目标
使用 YOLO 完成阿拉伯数字 0~9 的目标检测。本项目基于 ultralytics 源码环境，
以自采集/自制作的印刷体数字数据集进行训练，并在测试集上完成推理。

二、环境配置
1. 操作系统：Windows
2. Python：3.11（conda 环境名 yolo，路径 D:\Deeplearning\anaconda3\envs\yolo）
3. 核心依赖：
   ultralytics 8.3.x（从 D:\Deeplearning\ultralytics-8.3.163 源码目录运行）
   torch（本次训练使用 CPU，只能用cpu，/(ㄒoㄒ)/~~）
   numpy、opencv-python、pyyaml、matplotlib、pandas、seaborn
4. 预训练权重：yolo11n.pt
5. 数据集：D:\Deeplearning\ultralytics-8.3.163\datasets\number1\number1.yaml
    10 个类别（0~9）
   训练集 160 张，验证集 40 张（每类 16 训练 / 4 验证）
 是
三、运行方法
1. 训练（在 PyCharm 中打开项目 D:\Deeplearning\ultralytics-8.3.163，运行 mytrain.py）：
       python mytrain.py
   训练完成后权重保存在 runs\digit\train_full2\weights\best.pt / last.pt，
   曲线、混淆矩阵等图表也输出在 runs\digit\train_full2\。

2. 推理检测（运行 mypredict.py，把 source 指向待检测图片目录）：
       python mypredict.py
   检测结果图输出在 runs\predict_result\my_digit_test\（共 40 张），
   图上叠加了预测框、类别与置信度。

四、参数说明（本次实际训练参数， runs\digit\train_full2\args.yaml）
模型结构        yolo11n
训练轮数        epochs = 40
批次大小        batch = 4
输入尺寸        imgsz = 640
训练设备        device = cpu
预热轮数        warmup_epochs = 3
早停耐心值      patience = 15（验证集 15 轮无提升则提前停止）

五、实验结果
1. 关键指标（最终轮 epoch=40；最优 mAP50-95 出现在 epoch=38）
   Precision(B)      = 0.984  精确率
   Recall(B)         = 1.000	召回率
   mAP50(B)          = 0.995	平均精度均值
   mAP50-95(B)       = 0.980（最优 epoch 38）
   PR 曲线：10 个类别的 mAP50 全部为 0.995，PR 曲线紧贴左上角，说明在
   本验证集上分类与定位几乎完美。

2. Loss 曲线分析（results.png）
   val 三项损失（box/cls/dfl）同步下降，与训练损失趋势一致，
     没有出现"训练损失降、验证损失升"的发散，说明没有明显过拟合迹象
     但是这个数据集太简单


3 推理结果
    runs\predict_result\my_digit_test\ 下 40 张图均正确框出数字并给出
     类别与置信度（如 dig_7_16.jpg 检出 "7 0.86"），单字识别无误。

六、已知问题 、 失败情况分析（题目要求至少两类）

失败情况 1：泛化能力不足
   1.验证集指标接近满分，但训练用的是单一 字体、白底、
     居中的印刷数字。换成手写体、真实拍照背景、倾斜/粘连数字时，预期性能
     会明显下降
   2.样本量小（仅 200 张）、字体单一、无真实背景、无旋转/畸变
   3.改进：补充手写体、真实场景背景、多字体、多尺寸
     与随机旋转的数据；扩大训练集到数千张；引入多数字连写样本

失败情况2：对多个数字粘连识别效果不佳
   1.现连续数字（如“123”）且间距很近时，模型有时只能框出第一个数字，
后面的数字被漏检 
   2.训练集中可能缺乏这种极小尺寸的样本，导致模型对小目标泛化能力差
   3.针对性改进：提供多角度拍摄的图片数据集、有多个数字出现甚至粘连、连笔的数据集
   4.iou 阈值从默认的 0.7 降低到 0.4~0.5，防止相邻框被错误过滤
	
七、可改进方向
1. 扩充数据集规模与多样性（手写体 + 真实背景 + 连写数字），
   这是当前最瓶颈的一环；可直接复用项目内已生成的 digits_seq
   （真实背景手写连写）数据集。
2. 当前用 yolo11n（最小模型），在数据变难后可换 yolo11s/m
   提升容量。
3. 当前验证集仅 40 张，指标方差大；建议按 7:2:1 重新划分，
   并引入独立测试集，避免在小样本上"假性满分"。
4.当前数据集数量少，但是由于内存问题和电脑无法用GPU来跑模型，
   暂且选择了网上找到的数量、压缩包大小合适的数据集，运用到真实中需要更多
   的、且具有扭曲、手写等贴近生活的数字图片
	由于最近还在处理其他相关的，比如点云处理，电脑内存要booom了，所以
   数据集以及训练轮数都有偏少的问题，不是故意的🙏
   电脑卡机黑屏重启了好几次🙏 不敢乱来了🙏 希望理解🙏


附：相关路径
  训练脚本：  D:\Deeplearning\ultralytics-8.3.163\mytrain.py
  预测脚本：  D:\Deeplearning\ultralytics-8.3.163\mypredict.py
  训练输出：  D:\Deeplearning\ultralytics-8.3.163\runs\digit\train_full2\
  检测结果：  D:\Deeplearning\ultralytics-8.3.163\runs\predict_result\my_digit_test\
  数据集：    D:\Deeplearning\ultralytics-8.3.163\datasets\number1\number1.yaml

