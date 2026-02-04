# Contributing to OptiStream

Thank you for your interest in contributing to OptiStream! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in [Issues](https://github.com/arslanamjad19/OptiStream/issues)
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - System information (OS, compiler, library versions)
   - Relevant logs or error messages

### Suggesting Features

1. Check existing feature requests
2. Create an issue with:
   - Clear description of the feature
   - Use cases and benefits
   - Potential implementation approach

### Pull Requests

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Make your changes
4. Write/update tests
5. Update documentation
6. Commit with clear messages
7. Push to your fork
8. Open a pull request

## Development Setup

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential cmake \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libopencv-dev \
    python3-dev
```

### Building

```bash
./build.sh --debug --test
```

### Running Tests

```bash
cd build
ctest --output-on-failure
./tests/test_memory
```

## Code Style

### C++

- Follow C++17 standard
- Use meaningful variable names
- Keep functions focused and small
- Use RAII for resource management
- Prefer smart pointers over raw pointers
- Document public APIs with Doxygen comments

Example:
```cpp
/**
 * Process video frame
 * @param frame Input frame
 * @param timestamp Frame timestamp in seconds
 * @return true if processing succeeded
 */
bool process_frame(const cv::Mat& frame, double timestamp);
```

### Python

- Follow PEP 8 style guide
- Use type hints
- Write docstrings for public functions
- Keep functions focused

Example:
```python
def parse_output(data: np.ndarray, threshold: float = 0.5) -> List[Detection]:
    """
    Parse model output into detections.
    
    Args:
        data: Model output array
        threshold: Confidence threshold
        
    Returns:
        List of Detection objects
    """
    pass
```

## Testing Guidelines

### C++ Tests

- Use descriptive test names
- Test both success and failure cases
- Check for memory leaks
- Verify thread safety

### Python Tests

- Test with different input types
- Validate error handling
- Check edge cases

## Documentation

- Update README.md for major changes
- Update QUICKSTART.md for setup changes
- Add examples for new features
- Keep docstrings up to date

## Commit Messages

Use clear, descriptive commit messages:

```
Add zero-copy frame mapping for GStreamer

- Implement direct buffer mapping to cv::Mat
- Add performance benchmarks
- Update documentation with zero-copy details
```

## Performance Considerations

- Profile before optimizing
- Measure memory usage
- Test with realistic workloads
- Document performance characteristics

## Adding Dependencies

- Justify the need for new dependencies
- Consider cross-platform compatibility
- Document installation in README.md
- Update CMakeLists.txt

## Release Process

1. Update version in CMakeLists.txt
2. Update CHANGELOG.md
3. Tag release: `git tag -a v1.0.0 -m "Release 1.0.0"`
4. Push tags: `git push --tags`

## Questions?

- Open an issue for questions
- Join discussions in pull requests
- Check existing documentation

Thank you for contributing to OptiStream!
