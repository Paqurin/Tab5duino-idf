# Contributing to Tab5duino-IDF

Thank you for your interest in contributing to the Tab5duino-IDF framework! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Guidelines](#contributing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Testing Requirements](#testing-requirements)
- [Documentation](#documentation)
- [Community](#community)

## Code of Conduct

This project adheres to a code of conduct adapted from the [Contributor Covenant](https://www.contributor-covenant.org/). By participating, you are expected to uphold this code.

### Our Pledge

We pledge to make participation in our project a harassment-free experience for everyone, regardless of age, body size, disability, ethnicity, sex characteristics, gender identity and expression, level of experience, education, socio-economic status, nationality, personal appearance, race, religion, or sexual identity and orientation.

### Expected Behavior

- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

### Prerequisites

Before contributing, ensure you have:

- **Hardware**: M5Stack Tab5 device for testing
- **Software**: PlatformIO Core 6.0+ installed
- **Knowledge**: Basic understanding of ESP32-P4, Arduino API, and ESP-IDF
- **Tools**: Git, code editor (VS Code recommended)

### Repository Structure

```
Tab5duino-idf/
├── framework/
│   ├── cores/tab5duino/      # Arduino compatibility layer
│   ├── libraries/            # Framework libraries (LVGL, etc.)
│   ├── variants/m5tab5/      # Board-specific definitions
│   ├── components/           # ESP-IDF components
│   └── package.json          # Framework package metadata
├── examples/                 # Example projects and demos
├── builder/                  # PlatformIO build scripts
├── boards/                   # Board definitions (m5tab5.json)
├── docs/                     # Additional documentation
├── tests/                    # Unit and integration tests
└── tools/                    # Development and build tools
```

### Key Components

- **Arduino Layer** (`framework/cores/tab5duino/`): Arduino API implementation
- **HAL Interfaces** (`framework/cores/tab5duino/hal/`): Hardware abstraction
- **Board Definition** (`boards/m5tab5.json`): M5Stack Tab5 configuration  
- **Build System** (`builder/main.py`): PlatformIO integration
- **Examples** (`examples/`): Usage demonstrations and testing

## Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub
git clone https://github.com/yourusername/tab5duino-idf.git
cd tab5duino-idf
```

### 2. Development Environment

```bash
# Install PlatformIO CLI if not already installed
pip install platformio

# Create a development project to test changes
mkdir test-project
cd test-project
pio project init --board m5tab5 --project-option "framework=tab5duino-idf"
```

### 3. Link Local Framework

```bash
# In your test project, modify platformio.ini:
[env:m5tab5]
platform = file://../Tab5duino-idf
board = m5tab5
framework = tab5duino-idf
monitor_speed = 115200
```

### 4. Hardware Setup

- Connect M5Stack Tab5 via USB-C cable
- Ensure USB drivers are installed
- Verify device detection: `pio device list`

## Contributing Guidelines

### What Can You Contribute?

#### Framework Core
- Arduino API implementations and improvements
- HAL driver enhancements (display, touch, IMU, audio, power)
- Memory management optimizations
- Performance improvements
- Bug fixes and stability improvements

#### Libraries and Components
- New Arduino-compatible libraries
- LVGL integration improvements  
- ESP-IDF component wrappers
- Hardware-specific utilities

#### Documentation
- API documentation improvements
- Tutorial creation and enhancement
- Example project development
- Migration guides and best practices

#### Examples and Demos
- Hardware demonstration projects
- Performance benchmark examples
- Real-world application samples
- Educational content

#### Tools and Build System
- Build system optimizations
- Development tool improvements
- Testing framework enhancements
- CI/CD pipeline contributions

### Contribution Types

#### Bug Reports
- Use GitHub Issues with detailed reproduction steps
- Include hardware version, software versions, build logs
- Provide minimal reproduction case when possible

#### Feature Requests  
- Open GitHub Issues with detailed use case description
- Explain the problem being solved
- Consider implementation complexity and compatibility

#### Code Contributions
- Follow pull request process below
- Ensure comprehensive testing on hardware
- Update documentation as needed

## Pull Request Process

### 1. Preparation

```bash
# Create feature branch from main
git checkout -b feature/your-feature-name

# Make your changes
# Test thoroughly on hardware
```

### 2. Quality Checks

Before submitting a pull request:

- [ ] **Build Test**: Verify all examples compile successfully
- [ ] **Hardware Test**: Test on actual M5Stack Tab5 hardware  
- [ ] **Code Style**: Follow project coding standards
- [ ] **Documentation**: Update relevant documentation
- [ ] **Backwards Compatibility**: Ensure API compatibility
- [ ] **Performance**: Verify no performance regressions

### 3. Commit Guidelines

```bash
# Use conventional commit format
feat: add display brightness control API
fix: resolve touch calibration issue
docs: update HAL documentation
test: add IMU sensor validation
```

Commit message format:
- **feat**: New feature addition
- **fix**: Bug fix
- **docs**: Documentation changes
- **test**: Test additions or modifications
- **refactor**: Code refactoring without functional changes
- **perf**: Performance improvements
- **style**: Code style/formatting changes

### 4. Pull Request Submission

1. Push your branch: `git push origin feature/your-feature-name`
2. Create pull request on GitHub
3. Fill out the pull request template completely
4. Link related issues if applicable
5. Wait for review and address feedback

### 5. Review Process

- **Automated Checks**: Build verification and basic testing
- **Code Review**: Maintainer review for quality and compatibility  
- **Hardware Testing**: Verification on M5Stack Tab5 hardware
- **Documentation Review**: Ensure documentation is updated
- **Final Approval**: Merge after all requirements met

## Coding Standards

### C/C++ Style Guide

#### Naming Conventions
```cpp
// Functions and variables: snake_case
void initialize_display();
int sensor_value;

// Classes: PascalCase
class DisplayManager;

// Constants and macros: UPPER_CASE
#define MAX_BUFFER_SIZE 1024
const int DEFAULT_TIMEOUT = 5000;

// HAL functions: module_hal_action
esp_err_t display_hal_init();
esp_err_t touch_hal_read_points();
```

#### Code Structure
```cpp
// Header guard format
#ifndef _MODULE_NAME_H_
#define _MODULE_NAME_H_

// Include order: system, ESP-IDF, framework, local
#include <stdint.h>
#include "esp_err.h"
#include "framework_common.h"
#include "local_header.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
esp_err_t module_init(const config_t* config);

#ifdef __cplusplus
}
#endif

#endif // _MODULE_NAME_H_
```

#### Error Handling
```cpp
// Always use ESP-IDF error codes
esp_err_t function_name() {
    esp_err_t ret = ESP_OK;
    
    // Check parameters
    if (param == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Operation with error checking
    ret = some_operation();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Operation failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}
```

### Documentation Standards

#### Header Comments
```cpp
/**
 * @file display_hal.h
 * @brief Hardware abstraction layer for M5Stack Tab5 display
 * 
 * Provides high-level interface for 1280x720 MIPI-DSI display
 * with touch input and hardware acceleration support.
 */
```

#### Function Documentation
```cpp
/**
 * @brief Initialize display hardware
 * 
 * Configures MIPI-DSI interface and display controller for
 * M5Stack Tab5 1280x720 LCD panel.
 * 
 * @param config Display configuration structure
 * @param handle Output handle for display operations
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_hal_init(const display_config_t* config, display_handle_t* handle);
```

## Testing Requirements

### Unit Testing

```cpp
// Use Unity test framework
#include "unity.h"
#include "display_hal.h"

void test_display_init_success() {
    display_config_t config = {
        .width = 1280,
        .height = 720,
        .refresh_rate = 60
    };
    display_handle_t handle;
    
    TEST_ASSERT_EQUAL(ESP_OK, display_hal_init(&config, &handle));
}

void test_display_init_invalid_config() {
    display_handle_t handle;
    
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, display_hal_init(NULL, &handle));
}
```

### Integration Testing

```cpp
// Hardware integration test
void test_full_display_pipeline() {
    // Initialize display
    display_config_t config = DEFAULT_DISPLAY_CONFIG;
    display_handle_t display;
    TEST_ASSERT_EQUAL(ESP_OK, display_hal_init(&config, &display));
    
    // Test basic operations
    TEST_ASSERT_EQUAL(ESP_OK, display_hal_clear(display, 0x0000));
    TEST_ASSERT_EQUAL(ESP_OK, display_hal_draw_pixel(display, 100, 100, 0xFFFF));
    
    // Cleanup
    display_hal_deinit(display);
}
```

### Performance Testing

```cpp
void test_display_performance() {
    uint64_t start_time = esp_timer_get_time();
    
    // Perform operation
    for (int i = 0; i < 1000; i++) {
        display_hal_draw_pixel(display, i % 1280, i % 720, 0xFFFF);
    }
    
    uint64_t duration = esp_timer_get_time() - start_time;
    TEST_ASSERT_LESS_THAN(100000, duration); // Must complete in 100ms
}
```

### Hardware Testing

Always test on actual M5Stack Tab5 hardware:

1. **Build and Upload**: `pio run --target upload`
2. **Monitor Output**: `pio device monitor`
3. **Verify Functionality**: Test all modified features
4. **Performance Check**: Ensure no regressions
5. **Long-term Stability**: Extended operation test

## Documentation

### README Updates

When adding new features, update the main README.md:

- Add new APIs to the reference section
- Update examples if applicable
- Modify installation instructions if needed
- Update hardware requirements

### API Documentation

Document all public APIs with:

- Purpose and functionality
- Parameter descriptions
- Return value meanings
- Usage examples
- Error conditions
- Performance characteristics

### Example Projects

When creating examples:

- Include comprehensive `README.md`
- Provide clear build instructions
- Add detailed comments in code
- Test on hardware before submitting
- Keep examples focused and educational

## Community

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and community support
- **Discord**: Real-time discussion (link in main README)
- **Email**: Direct contact for security issues

### Getting Help

- Check existing documentation and examples first
- Search GitHub Issues for similar problems
- Post detailed questions in GitHub Discussions
- Include hardware details, software versions, and error messages

### Maintainer Guidelines

For project maintainers:

- Review pull requests within 7 days
- Provide constructive feedback
- Test on hardware when possible
- Update documentation as needed
- Communicate clearly with contributors

## Release Process

### Version Management

- Follow semantic versioning (MAJOR.MINOR.PATCH)
- Update version in `platform.json` and `framework/package.json`
- Create detailed CHANGELOG entries
- Tag releases properly

### Quality Gates

Before release:
- [ ] All examples build successfully
- [ ] Hardware testing completed
- [ ] Documentation updated
- [ ] Performance benchmarks verified
- [ ] Backwards compatibility maintained

## Recognition

Contributors will be recognized through:

- GitHub contributor statistics
- CHANGELOG acknowledgments  
- Project documentation credits
- Community recognition

Thank you for contributing to Tab5duino-IDF and helping make ESP32-P4 development more accessible and powerful!