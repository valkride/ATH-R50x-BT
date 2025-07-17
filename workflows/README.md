# GitHub Actions CI/CD for ATH-R50x Firmware

This directory contains GitHub Actions workflows for automatically building and releasing the ATH-R50x Bluetooth headset firmware.

## Available Workflows

### 1. `build-platformio.yml` - PlatformIO Build
- **Triggers**: Push to `main`/`develop`, Pull requests to `main`
- **Purpose**: Builds firmware using PlatformIO (Arduino framework)
- **Artifacts**: Firmware binaries, ELF files, bootloader, partitions
- **Auto-release**: Creates releases on version tags

### 2. `build-espidf.yml` - ESP-IDF Build
- **Triggers**: Push to `main`/`develop` (Build/ folder changes), Pull requests to `main`
- **Purpose**: Builds firmware using native ESP-IDF
- **Artifacts**: ESP-IDF compiled binaries
- **Auto-release**: Creates releases on version tags

### 3. `release.yml` - Release Build
- **Triggers**: Version tags (v1.0.0, v1.1.0, etc.)
- **Purpose**: Creates comprehensive release packages
- **Includes**: Firmware, flash scripts, documentation
- **Output**: GitHub release with all necessary files

### 4. `manual-build.yml` - Manual Build Trigger
- **Triggers**: Manual workflow dispatch
- **Purpose**: On-demand builds with custom options
- **Options**: PlatformIO, ESP-IDF, or both
- **Features**: Debug level selection, optional release package

## How to Use

### Automatic Builds
1. Push code to `main` or `develop` branch
2. GitHub Actions will automatically build the firmware
3. Download artifacts from the Actions tab

### Manual Builds
1. Go to the "Actions" tab in your GitHub repository
2. Select "Manual Build" workflow
3. Click "Run workflow"
4. Choose build type and options
5. Download the resulting artifacts

### Creating Releases
1. Create and push a version tag:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
2. GitHub Actions will automatically:
   - Build the firmware
   - Create flash scripts
   - Package everything
   - Create a GitHub release

## Build Artifacts

### PlatformIO Build Output
- `firmware.bin` - Main firmware binary
- `firmware.elf` - ELF file with debug symbols
- `partitions.bin` - Partition table
- `bootloader.bin` - ESP32-C3 bootloader

### ESP-IDF Build Output
- `bt_headset_module.bin` - Main firmware binary
- `bt_headset_module.elf` - ELF file with debug symbols
- `partition-table.bin` - Partition table
- `bootloader.bin` - ESP32-C3 bootloader

### Release Package
- Renamed firmware files with version
- Flash scripts for Windows and Linux/Mac
- README with installation instructions
- All necessary binaries

## Setting Up in Your Repository

1. **Push these files to your repository**:
   ```bash
   git add .github/
   git commit -m "Add GitHub Actions workflows"
   git push origin main
   ```

2. **Enable GitHub Actions**:
   - Go to your repository settings
   - Navigate to "Actions" section
   - Enable GitHub Actions if not already enabled

3. **Test the setup**:
   - Push a commit to trigger automatic build
   - Or manually trigger a build from Actions tab

## Customization

### Changing Build Options
Edit the workflow files to modify:
- Target board/chip
- Build flags
- Library dependencies
- Artifact retention time

### Adding New Triggers
You can add more triggers like:
- Scheduled builds (cron)
- Issue/PR comments
- External webhook triggers

### Custom Build Scripts
Add your own build steps by modifying the workflow files in the `.github/workflows/` directory.

## Troubleshooting

### Common Issues
1. **PlatformIO build fails**: Check library dependencies in `platformio.ini`
2. **ESP-IDF build fails**: Verify ESP-IDF version compatibility
3. **No artifacts**: Check build logs for compilation errors

### Debug Tips
- Enable debug output in workflow files
- Use manual builds to test changes
- Check the Actions tab for detailed logs

## Security Notes

- Workflows use official GitHub Actions and trusted ESP-IDF actions
- No sensitive information should be committed to workflows
- Use repository secrets for any sensitive configuration
