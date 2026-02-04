# Models Directory

Place your ONNX models in this directory.

## Supported Models

OptiStream supports any ONNX format model. For best results, use models with:
- Input format: `[batch, channels, height, width]` (NCHW)
- Input type: float32
- BGR or RGB color space

## Example Models

### YOLO (Object Detection)
Download pre-trained YOLO models:
- YOLOv5: https://github.com/ultralytics/yolov5
- YOLOv8: https://github.com/ultralytics/ultralytics

Export to ONNX:
```bash
pip install ultralytics
yolo export model=yolov8n.pt format=onnx
```

### Other Popular Models
- ResNet (Image Classification)
- MobileNet (Image Classification)
- EfficientDet (Object Detection)
- SegFormer (Segmentation)

## Directory Structure

```
models/
├── README.md          (this file)
├── yolov5s.onnx       (example YOLO model)
├── resnet50.onnx      (example classifier)
└── custom_model.onnx  (your custom models)
```

## Converting Models to ONNX

### PyTorch
```python
import torch
model = YourModel()
dummy_input = torch.randn(1, 3, 640, 640)
torch.onnx.export(model, dummy_input, "model.onnx")
```

### TensorFlow
```python
import tf2onnx
import onnx
model = tf.keras.models.load_model('model.h5')
onnx_model = tf2onnx.convert.from_keras(model)
onnx.save(onnx_model, "model.onnx")
```

## Model Requirements

- Format: ONNX (Open Neural Network Exchange)
- Input: Typically `[1, 3, H, W]` for RGB/BGR images
- Supported ops: See ONNX Runtime documentation
