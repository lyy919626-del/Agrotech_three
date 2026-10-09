# from ultralytics import YOLO
#
# model = YOLO(r"D:\Deeplearning\ultralytics-8.3.163\runs\detect\train28\weights\best.pt")
# model.predict(
#     source=r"D:\Deeplearning\make_dataset\images",
#     save=True,
#     show=False,
#     save_txt=True,
# )

from ultralytics import YOLO
import os

if __name__ == "__main__":
    # 1. 加载刚刚训练好的最佳权重 (注意确认路径是否正确)
    # 路径通常是你训练结果文件夹下的 weights/best.pt
    model = YOLO(r"runs/digit/train_full/weights/best.pt")

    # 2. 指定预测图片的来源
    # 最好用你没有参与训练的测试图片，或者验证集图片
    # 这里以你的验证集为例
    source_path = r"D:\Deeplearning\ultralytics-8.3.163\datasets\number1\images\val"

    # 3. 执行推理
    results = model.predict(
        source=source_path,
        save=True,  # 保存带有预测框的图片
        conf=0.25,  # 置信度阈值
        project='runs/predict_result',
        name='my_digit_test'
    )