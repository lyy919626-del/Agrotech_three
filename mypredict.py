from ultralytics import YOLO
import os

if __name__ == "__main__":
    # 1. 加载刚刚训练好的最佳权重 (注意确认路径是否正确)
    # 路径通常是训练结果文件夹下的 weights/best.pt
    model = YOLO(r"runs/digit/train_full/weights/best.pt")

    # 2. 指定预测图片的来源
    # 
    # 这里以验证集为例
    source_path = r"D:\Deeplearning\ultralytics-8.3.163\datasets\number1\images\val"

    # 3. 执行推理
    results = model.predict(
        source=source_path,
        save=True,  # 保存带有预测框的图片
        conf=0.25,  # 置信度阈值
        project='runs/predict_result',
        name='my_digit_test'
    )
