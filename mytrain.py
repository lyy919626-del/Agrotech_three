from ultralytics import YOLO

if __name__ == "__main__":
    model = YOLO(r"yolo11n.pt")
    model.train(
        data=r"tomatoes.yaml",  # 用来训练的数据集
        epochs=40,                  #训练多少轮
        imgsz=640,
        batch=4,                    #训练批量（一批里面有多少张图片）
        cache=False,                #缓存
        workers=2,                  #数据打包
        project="runs",             #指定总目录
        name="tomatoes_train"       #指定子文件夹
    )
