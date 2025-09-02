#!/bin/bash

# Make script executable
chmod +x "$0"

# Tab5duino-IDF v1.0.0 Release Package Generator
# Creates production-ready GitHub repository

set -e  # Exit on any error

echo "========================================"
echo "  Tab5duino-IDF v1.0.0 Release Package"  
echo "========================================"

FRAMEWORK_DIR="$(pwd)"
RELEASE_VERSION="v1.0.0"
REPOSITORY_URL="https://github.com/Paqurin/Tab5duino-idf.git"

# Color output for better visibility
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

echo_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

echo_error() {
    echo -e "${RED}✗ $1${NC}"
}

echo_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check if we're in the right directory
if [[ ! -f "platform.json" ]] || [[ ! -d "framework" ]]; then
    echo_error "Not in Tab5duino-IDF root directory!"
    echo_info "Please run this script from the Tab5duino-IDF root directory"
    exit 1
fi

echo_info "Framework directory: ${FRAMEWORK_DIR}"

# Step 1: Validate framework structure
echo
echo "Step 1: Validating framework structure..."

required_files=(
    "platform.json"
    "framework/package.json"
    "framework/cores/tab5duino/Arduino.h"
    "boards/m5tab5.json"
    "builder/main.py"
    "README.md"
    "LICENSE"
    "CHANGELOG.md"
    "CONTRIBUTING.md"
    "INSTALL.md"
    ".gitignore"
)

required_dirs=(
    "framework/cores/tab5duino"
    "framework/libraries"
    "framework/variants/m5tab5"
    "examples"
    ".github"
)

# Check required files
for file in "${required_files[@]}"; do
    if [[ -f "$file" ]]; then
        echo_success "Found: $file"
    else
        echo_error "Missing: $file"
        exit 1
    fi
done

# Check required directories
for dir in "${required_dirs[@]}"; do
    if [[ -d "$dir" ]]; then
        echo_success "Found directory: $dir"
    else
        echo_error "Missing directory: $dir"
        exit 1
    fi
done

# Step 2: Validate JSON files
echo
echo "Step 2: Validating JSON configuration..."

validate_json() {
    local file=$1
    if python3 -m json.tool "$file" > /dev/null 2>&1; then
        echo_success "Valid JSON: $file"
    else
        echo_error "Invalid JSON: $file"
        exit 1
    fi
}

validate_json "platform.json"
validate_json "framework/package.json"
validate_json "boards/m5tab5.json"

# Check version consistency
PLATFORM_VERSION=$(python3 -c "import json; print(json.load(open('platform.json'))['version'])")
FRAMEWORK_VERSION=$(python3 -c "import json; print(json.load(open('framework/package.json'))['version'])")

if [[ "$PLATFORM_VERSION" == "$FRAMEWORK_VERSION" ]]; then
    echo_success "Version consistency: $PLATFORM_VERSION"
else
    echo_error "Version mismatch: Platform=$PLATFORM_VERSION, Framework=$FRAMEWORK_VERSION"
    exit 1
fi

# Step 3: Validate examples
echo
echo "Step 3: Validating example projects..."

example_dirs=(
    "examples/HelloTab5"
    "examples/tab5_hardware_test"
    "examples/lvgl_basic"
)

for example in "${example_dirs[@]}"; do
    if [[ -d "$example" ]]; then
        if [[ -f "$example/platformio.ini" ]]; then
            echo_success "Example project: $example"
        else
            echo_warning "Missing platformio.ini in: $example"
        fi
    else
        echo_warning "Missing example: $example"
    fi
done

# Step 4: Initialize Git repository (if not already initialized)
echo
echo "Step 4: Preparing Git repository..."

if [[ ! -d ".git" ]]; then
    echo_info "Initializing Git repository..."
    git init
    echo_success "Git repository initialized"
else
    echo_info "Git repository already exists"
fi

# Configure Git if needed
if [[ -z "$(git config --get user.name)" ]]; then
    echo_warning "Git user.name not configured"
    echo_info "Please configure: git config user.name 'Your Name'"
fi

if [[ -z "$(git config --get user.email)" ]]; then
    echo_warning "Git user.email not configured"  
    echo_info "Please configure: git config user.email 'your.email@example.com'"
fi

# Step 5: Add files to Git
echo
echo "Step 5: Staging files for Git..."

# Add all framework files
git add .

# Check git status
git_status=$(git status --porcelain)
if [[ -n "$git_status" ]]; then
    echo_success "Files staged for commit"
    echo_info "Staged files:"
    git status --short | head -20
    if [[ $(git status --short | wc -l) -gt 20 ]]; then
        echo_info "... and $(( $(git status --short | wc -l) - 20 )) more files"
    fi
else
    echo_info "No changes to stage"
fi

# Step 6: Create initial commit (if needed)
echo
echo "Step 6: Creating initial commit..."

if [[ -z "$(git log --oneline 2>/dev/null)" ]]; then
    echo_info "Creating initial commit..."
    git commit -m "feat: initial release of Tab5duino-IDF v1.0.0

- Arduino-compatible framework for ESP32-P4 M5Stack Tab5
- Built on ESP-IDF foundation for stability and performance  
- Complete hardware abstraction layer (HAL) implementation
- LVGL integration with optimized configuration
- Comprehensive example projects and documentation
- Professional build system with PlatformIO integration
- Full 32MB PSRAM support and hardware acceleration ready
- MIT license for commercial and open-source use"

    echo_success "Initial commit created"
else
    echo_info "Repository already has commits"
    if [[ -n "$git_status" ]]; then
        echo_info "Creating release preparation commit..."
        git commit -m "docs: prepare for v1.0.0 release

- Add comprehensive documentation (README, CHANGELOG, CONTRIBUTING)
- Create professional GitHub templates and metadata
- Add installation guide and migration documentation
- Finalize release package with all required files"
        echo_success "Release preparation commit created"
    fi
fi

# Step 7: Create version tag
echo
echo "Step 7: Creating release tag..."

if git tag | grep -q "^${RELEASE_VERSION}$"; then
    echo_info "Tag $RELEASE_VERSION already exists"
else
    echo_info "Creating tag: $RELEASE_VERSION"
    git tag -a "$RELEASE_VERSION" -m "Tab5duino-IDF v1.0.0 Release

Arduino-compatible framework for ESP32-P4 M5Stack Tab5 built on ESP-IDF foundation.

Features:
- Complete Arduino API compatibility layer
- ESP-IDF foundation for stability and performance
- Hardware abstraction layer for M5Stack Tab5 components
- LVGL integration with optimized configuration  
- Comprehensive documentation and examples
- Professional build system integration
- 32MB PSRAM support with hardware acceleration
- MIT license for open source and commercial use

This is the initial stable release ready for production use."

    echo_success "Release tag created: $RELEASE_VERSION"
fi

# Step 8: Validate final package
echo
echo "Step 8: Final package validation..."

# Check file sizes to ensure reasonable package size
total_size=$(du -sh . | cut -f1)
echo_info "Total package size: $total_size"

# Check for large files that shouldn't be in the repository
large_files=$(find . -type f -size +1M -not -path "./.git/*" 2>/dev/null || true)
if [[ -n "$large_files" ]]; then
    echo_warning "Large files found (>1MB):"
    echo "$large_files"
    echo_info "Consider if these files should be in the repository"
fi

# Generate package summary
echo
echo "========================================" 
echo "  Release Package Summary"
echo "========================================"

echo_info "Framework: Tab5duino-IDF"
echo_info "Version: $RELEASE_VERSION"
echo_info "Target: ESP32-P4 M5Stack Tab5"
echo_info "License: MIT"
echo_info "Package Size: $total_size"

file_count=$(find . -type f -not -path "./.git/*" | wc -l)
echo_info "Files: $file_count"

example_count=$(find examples -name "platformio.ini" | wc -l)
echo_info "Example Projects: $example_count"

echo_info "Repository: $REPOSITORY_URL"

echo
echo_success "Release package preparation complete!"

# Step 9: Next steps information
echo
echo "========================================" 
echo "  Next Steps"
echo "========================================"

echo_info "1. Review the staged changes:"
echo "   git log --oneline -5"
echo "   git show $RELEASE_VERSION"

echo
echo_info "2. Push to GitHub repository:"
echo "   git remote add origin $REPOSITORY_URL"
echo "   git push -u origin master"
echo "   git push origin $RELEASE_VERSION"

echo
echo_info "3. Create GitHub Release:"
echo "   - Go to: $REPOSITORY_URL/releases"
echo "   - Click 'Create a new release'"
echo "   - Select tag: $RELEASE_VERSION"
echo "   - Title: Tab5duino-IDF v1.0.0"
echo "   - Copy description from CHANGELOG.md"
echo "   - Upload any additional assets if needed"

echo
echo_info "4. Verify installation:"
echo "   pio platform install $REPOSITORY_URL"
echo "   pio project init --board m5tab5 --project-option \"framework=tab5duino-idf\""

echo
echo_info "5. Update documentation links:"
echo "   - Update README.md URLs if repository location differs"
echo "   - Update platform.json homepage and repository URLs" 
echo "   - Update framework/package.json repository URL"

echo
echo_success "Tab5duino-IDF v1.0.0 is ready for professional GitHub release!"
echo_info "Repository: $REPOSITORY_URL"