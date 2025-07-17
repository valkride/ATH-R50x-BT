# Security Policy

## Supported Versions

We actively support the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it to us privately:

### How to Report

1. **Email**: Send details to security@yourproject.com
2. **GitHub**: Create a private security advisory
3. **Include**: 
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if known)

### Response Time

- **Initial Response**: Within 48 hours
- **Investigation**: Within 1 week
- **Fix Timeline**: Depends on severity
- **Disclosure**: After fix is released

### Security Considerations

This firmware handles:
- **Bluetooth communications** (potential pairing vulnerabilities)
- **Audio data** (privacy considerations)
- **USB HID** (potential keystroke injection)
- **Network interfaces** (if WiFi is enabled)

Please report any security issues responsibly.

## Security Best Practices

When using this firmware:

1. **Keep firmware updated** to latest version
2. **Use secure pairing** methods
3. **Disable unused features** to reduce attack surface
4. **Monitor for suspicious behavior**
5. **Follow hardware security guidelines**

## Known Security Considerations

- **Bluetooth pairing**: Uses standard pairing methods
- **USB HID**: Can send keystrokes (by design for mute function)
- **No encryption**: Audio data is not encrypted in transit
- **Debug interfaces**: Disable in production builds

## Updates

Security updates will be released as:
- **Critical**: Immediate patch release
- **High**: Next minor version
- **Medium/Low**: Next major version

Check releases regularly for security updates.
