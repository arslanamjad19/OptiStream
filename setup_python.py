#!/usr/bin/env python3
"""
Simple setup helper for OptiStream Python bindings
"""

import os
import sys
import subprocess
from pathlib import Path

def main():
    """Check dependencies and provide setup instructions"""
    
    print("OptiStream Python Setup Helper")
    print("=" * 50)
    print()
    
    # Check Python version
    print(f"Python version: {sys.version}")
    if sys.version_info < (3, 7):
        print("❌ Python 3.7 or higher is required")
        return 1
    else:
        print("✓ Python version OK")
    print()
    
    # Check for required packages
    packages = {
        'numpy': 'NumPy',
        'cv2': 'OpenCV',
    }
    
    missing = []
    for package, name in packages.items():
        try:
            __import__(package)
            print(f"✓ {name} installed")
        except ImportError:
            print(f"❌ {name} not installed")
            missing.append(package if package != 'cv2' else 'opencv-python')
    
    print()
    
    if missing:
        print("Install missing packages with:")
        print(f"  pip install {' '.join(missing)}")
        print()
    
    # Check for OptiStream module
    try:
        import optistream
        print("✓ OptiStream module found")
        print(f"  Location: {optistream.__file__}")
    except ImportError:
        print("❌ OptiStream module not found")
        print()
        print("Build the project first:")
        print("  ./build.sh")
        print()
        print("Then add to PYTHONPATH:")
        build_path = Path(__file__).parent / "build" / "python"
        print(f"  export PYTHONPATH={build_path}:$PYTHONPATH")
        print()
        return 1
    
    print()
    print("=" * 50)
    print("Setup complete! You can now run the examples:")
    print("  python examples/demo_yolo.py --help")
    print()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
