# Contributing to ESP32-C3 SuperMini Bluetooth Headset

We welcome contributions to this project! This document provides guidelines for contributing.

## 🎯 Ways to Contribute

- **Bug Reports**: Report issues you encounter
- **Feature Requests**: Suggest new features or improvements
- **Code Contributions**: Submit pull requests with bug fixes or new features
- **Documentation**: Improve README, comments, or add examples
- **Testing**: Test on different hardware configurations

## 🚀 Getting Started

### Development Environment Setup

1. **Install PlatformIO:**
```bash
pip install platformio
```

2. **Clone the repository:**
```bash
git clone https://github.com/yourusername/esp32-c3-bluetooth-headset.git
cd esp32-c3-bluetooth-headset
```

3. **Install dependencies:**
```bash
pio run
```

### Building and Testing

1. **Build the project:**
```bash
pio run
```

2. **Run tests:**
```bash
pio test
```

3. **Flash to hardware:**
```bash
pio run --target upload
```

## 📋 Contribution Guidelines

### Code Style

- **C++ Standard**: Follow C++11/14 standards
- **Naming**: Use camelCase for variables, PascalCase for classes
- **Indentation**: 4 spaces (no tabs)
- **Comments**: Document all public functions and complex logic
- **Constants**: Use `#define` for pin definitions, `const` for other constants

### File Organization

- **Source files**: Place in `src/` directory
- **Libraries**: Create in `lib/` directory
- **Headers**: Use `include/` for global headers
- **Documentation**: Add to `docs/` directory
- **Examples**: Place in `examples/` directory

### Git Workflow

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Commit** your changes: `git commit -m 'Add amazing feature'`
4. **Push** to the branch: `git push origin feature/amazing-feature`
5. **Open** a Pull Request

### Commit Messages

Use clear, descriptive commit messages:
```
feat: add voice activity detection to audio processor
fix: resolve I2C communication timeout issue
docs: update pin configuration documentation
refactor: improve battery monitoring efficiency
test: add unit tests for QCC5124 control
```

## 🐛 Bug Reports

When reporting bugs, please include:

- **ESP32-C3 board variant** (SuperMini, DevKitM-1, etc.)
- **PlatformIO version** and environment
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Serial output** or error messages
- **Hardware configuration** and connections

### Bug Report Template

```markdown
**Board**: ESP32-C3 SuperMini
**PlatformIO Version**: 6.1.0
**Framework**: Arduino

**Description**: 
Brief description of the bug

**Steps to Reproduce**:
1. Step 1
2. Step 2
3. Step 3

**Expected Behavior**: 
What should happen

**Actual Behavior**: 
What actually happens

**Serial Output**:
```
Paste serial output here
```

**Hardware Configuration**:
- Pin connections
- External components
- Power supply details
```

## 💡 Feature Requests

For new features, please describe:

- **Use case**: Why is this feature needed?
- **Implementation**: How should it work?
- **Hardware requirements**: Any additional components needed?
- **Compatibility**: Impact on existing functionality

## 🔧 Development Areas

### High Priority
- **Audio Quality**: Improve noise suppression algorithms
- **Power Efficiency**: Optimize battery life
- **Hardware Support**: Add support for more codec variants
- **Testing**: Expand test coverage

### Medium Priority
- **UI/UX**: Improve OLED display layouts
- **Configuration**: Add runtime configuration options
- **Documentation**: More detailed hardware guides
- **Examples**: Additional usage examples

### Low Priority
- **Advanced Features**: OTA updates, web interface
- **Platform Support**: Other ESP32 variants
- **Integration**: Home Assistant, etc.

## 📚 Documentation

### Code Documentation

- **Function comments**: Describe purpose, parameters, return values
- **Class documentation**: Overview of functionality
- **Complex algorithms**: Step-by-step explanation
- **Hardware interfaces**: Pin descriptions and timing

### User Documentation

- **Setup guides**: Hardware assembly instructions
- **Troubleshooting**: Common issues and solutions
- **API reference**: Function and class documentation
- **Examples**: Complete working examples

## 🧪 Testing

### Unit Tests

- **Create tests** for new functions
- **Test edge cases** and error conditions
- **Mock hardware** interfaces for testing
- **Verify backwards compatibility**

### Hardware Testing

- **Test on actual hardware** before submitting
- **Verify pin configurations** and connections
- **Check power consumption** and battery life
- **Test audio quality** and latency

## 🔍 Code Review Process

All contributions go through code review:

1. **Automated checks**: Code style, build, tests
2. **Manual review**: Functionality, design, documentation
3. **Hardware testing**: If applicable
4. **Documentation review**: Ensure completeness

### Review Criteria

- **Functionality**: Does it work as intended?
- **Code quality**: Is it well-structured and readable?
- **Performance**: Are there any efficiency issues?
- **Compatibility**: Does it break existing functionality?
- **Documentation**: Is it properly documented?

## 📞 Getting Help

### Discussion Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and community support
- **Wiki**: Detailed documentation and guides

### Before Asking

1. **Check existing issues** and discussions
2. **Review documentation** and examples
3. **Test with minimal configuration**
4. **Provide complete information**

## 🎖️ Recognition

Contributors are recognized in:

- **README.md**: Acknowledgments section
- **CHANGELOG.md**: Feature and fix credits
- **GitHub**: Contributor graph and statistics

## 📋 Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Testing
- [ ] Tested on hardware
- [ ] Unit tests pass
- [ ] No regression in existing functionality

## Hardware Configuration
- Board: ESP32-C3 SuperMini
- Components: List any specific hardware used

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] No breaking changes (or documented)
```

## 🏆 Code of Conduct

Be respectful and professional in all interactions. We're all here to build something great together!

---

Thank you for contributing to the ESP32-C3 SuperMini Bluetooth Headset project! 🎧
