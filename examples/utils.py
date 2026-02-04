"""
OptiStream Python utilities
"""
import numpy as np
from typing import List, Tuple

def non_max_suppression(boxes: np.ndarray, scores: np.ndarray, 
                        iou_threshold: float = 0.5) -> List[int]:
    """
    Apply Non-Maximum Suppression to bounding boxes
    
    Args:
        boxes: Array of bounding boxes (N, 4) in format [x1, y1, x2, y2]
        scores: Array of confidence scores (N,)
        iou_threshold: IoU threshold for suppression
        
    Returns:
        List of indices to keep
    """
    if len(boxes) == 0:
        return []
    
    # Sort by scores
    indices = np.argsort(scores)[::-1]
    
    keep = []
    while len(indices) > 0:
        # Pick the box with highest score
        current = indices[0]
        keep.append(current)
        
        if len(indices) == 1:
            break
            
        # Compute IoU with remaining boxes
        current_box = boxes[current]
        remaining_boxes = boxes[indices[1:]]
        
        # Calculate intersection
        x1 = np.maximum(current_box[0], remaining_boxes[:, 0])
        y1 = np.maximum(current_box[1], remaining_boxes[:, 1])
        x2 = np.minimum(current_box[2], remaining_boxes[:, 2])
        y2 = np.minimum(current_box[3], remaining_boxes[:, 3])
        
        intersection = np.maximum(0, x2 - x1) * np.maximum(0, y2 - y1)
        
        # Calculate union
        current_area = (current_box[2] - current_box[0]) * (current_box[3] - current_box[1])
        remaining_areas = (remaining_boxes[:, 2] - remaining_boxes[:, 0]) * \
                         (remaining_boxes[:, 3] - remaining_boxes[:, 1])
        union = current_area + remaining_areas - intersection
        
        # Calculate IoU
        iou = intersection / union
        
        # Keep boxes with IoU less than threshold
        indices = indices[1:][iou < iou_threshold]
    
    return keep


def parse_yolo_output(output: np.ndarray, 
                     conf_threshold: float = 0.25,
                     iou_threshold: float = 0.45,
                     img_width: int = 640,
                     img_height: int = 640) -> Tuple[List, List, List]:
    """
    Parse YOLO model output into bounding boxes, scores, and class IDs
    
    Args:
        output: Raw model output (1, 25200, 85) for YOLOv5
                or similar format for other YOLO versions
        conf_threshold: Confidence threshold
        iou_threshold: IoU threshold for NMS
        img_width: Original image width
        img_height: Original image height
        
    Returns:
        Tuple of (boxes, scores, class_ids)
    """
    # Handle different output formats
    if len(output.shape) == 3:
        output = output[0]  # Remove batch dimension
    
    # Assuming output shape is (num_predictions, 85) for COCO dataset
    # [x, y, w, h, objectness, class1, class2, ..., class80]
    if output.shape[1] >= 5:
        boxes_xywh = output[:, :4]
        objectness = output[:, 4]
        
        # Get class predictions
        if output.shape[1] > 5:
            class_probs = output[:, 5:]
            class_ids = np.argmax(class_probs, axis=1)
            class_scores = np.max(class_probs, axis=1)
            scores = objectness * class_scores
        else:
            scores = objectness
            class_ids = np.zeros(len(scores), dtype=int)
        
        # Filter by confidence threshold
        mask = scores > conf_threshold
        boxes_xywh = boxes_xywh[mask]
        scores = scores[mask]
        class_ids = class_ids[mask]
        
        # Convert from [x_center, y_center, w, h] to [x1, y1, x2, y2]
        boxes = np.zeros_like(boxes_xywh)
        boxes[:, 0] = boxes_xywh[:, 0] - boxes_xywh[:, 2] / 2  # x1
        boxes[:, 1] = boxes_xywh[:, 1] - boxes_xywh[:, 3] / 2  # y1
        boxes[:, 2] = boxes_xywh[:, 0] + boxes_xywh[:, 2] / 2  # x2
        boxes[:, 3] = boxes_xywh[:, 1] + boxes_xywh[:, 3] / 2  # y2
        
        # Apply NMS
        keep_indices = non_max_suppression(boxes, scores, iou_threshold)
        
        boxes = boxes[keep_indices]
        scores = scores[keep_indices]
        class_ids = class_ids[keep_indices]
        
        # Scale to image dimensions
        boxes[:, [0, 2]] *= img_width
        boxes[:, [1, 3]] *= img_height
        
        return boxes.tolist(), scores.tolist(), class_ids.tolist()
    
    return [], [], []


def draw_boxes(image: np.ndarray, 
               boxes: List, 
               scores: List, 
               class_ids: List,
               class_names: List[str] = None) -> np.ndarray:
    """
    Draw bounding boxes on image
    
    Args:
        image: Input image (H, W, 3)
        boxes: List of boxes [x1, y1, x2, y2]
        scores: List of confidence scores
        class_ids: List of class IDs
        class_names: Optional list of class names
        
    Returns:
        Image with drawn boxes
    """
    import cv2
    
    result = image.copy()
    
    for box, score, class_id in zip(boxes, scores, class_ids):
        x1, y1, x2, y2 = map(int, box)
        
        # Draw rectangle
        cv2.rectangle(result, (x1, y1), (x2, y2), (0, 255, 0), 2)
        
        # Prepare label
        if class_names and class_id < len(class_names):
            label = f"{class_names[class_id]}: {score:.2f}"
        else:
            label = f"Class {class_id}: {score:.2f}"
        
        # Draw label background
        (label_width, label_height), _ = cv2.getTextSize(
            label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
        cv2.rectangle(result, (x1, y1 - label_height - 10), 
                     (x1 + label_width, y1), (0, 255, 0), -1)
        
        # Draw label text
        cv2.putText(result, label, (x1, y1 - 5), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    
    return result


# COCO dataset class names
COCO_CLASSES = [
    'person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat',
    'traffic light', 'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird', 'cat',
    'dog', 'horse', 'sheep', 'cow', 'elephant', 'bear', 'zebra', 'giraffe', 'backpack',
    'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee', 'skis', 'snowboard', 'sports ball',
    'kite', 'baseball bat', 'baseball glove', 'skateboard', 'surfboard', 'tennis racket',
    'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
    'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza', 'donut', 'cake', 'chair',
    'couch', 'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote',
    'keyboard', 'cell phone', 'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book',
    'clock', 'vase', 'scissors', 'teddy bear', 'hair drier', 'toothbrush'
]
