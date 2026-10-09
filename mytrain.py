from ultralytics import YOLO

if __name__ == "__main__":
    model = YOLO(r"yolo11n.pt")
    model.train(
        data=r"D:\Deeplearning\ultralytics-8.3.163\datasets\number1\number1.yaml",  # 用来训练的数据集
        epochs=40,                  #训练多少轮
        imgsz=640,
        batch=4,                    #训练批量（一批里面有多少张图片）
        cache=False,                #缓存
        workers=2,                  #数据打包
        project='runs/digit',       # 【新增】指定一个专属文件夹
        name='train_full',       # 【新增】命名为 train_full
        patience=15   # 【新增】早停机制，如果20轮没有提升就自动停止
    )
