# Vista SDK Python Documentation

Welcome to the comprehensive documentation for the Vista SDK Python implementation!

## 📖 Documentation Overview

This directory contains complete documentation for using the Vista SDK Python library to work with vessel data following DNV's Vessel Information Structure (VIS) standard and ISO 19847/19848.

## 📚 Available Guides

### 🚀 [Getting Started](getting-started.md)
**Perfect for newcomers** - Start here if you're new to the Vista SDK or VIS standards.
- Quick 5-minute setup and first Local ID creation
- Core concepts explained simply
- Common use cases with examples
- Development setup and troubleshooting
- Learning path recommendations

### 🎓 [Tutorials](tutorials.md)
**Step-by-step learning** - Comprehensive tutorials covering all major features.
- 14 detailed tutorials from basic to advanced
- Working with GMOD equipment hierarchy
- Building and parsing Local IDs
- Using codebooks for metadata validation
- Version management and conversions
- Integration patterns and best practices

### 📋 [API Reference](API.md)
**Complete technical reference** - Detailed documentation of all classes and methods.
- All classes, methods, and properties documented
- Parameters, return types, and exceptions
- Code examples for every major feature
- Performance notes and threading information
- Migration guide from C# SDK

## 🎯 Quick Navigation

### I want to...

**🏃‍♂️ Get started quickly**
→ [Getting Started Guide](getting-started.md) - 5-minute quickstart

**📖 Learn step by step**
→ [Tutorials](tutorials.md) - Complete learning path

**🔍 Look up specific methods**
→ [API Reference](API.md) - Technical documentation

**💡 See practical examples**
→ [../samples/](../samples/) - Working code examples

**🐛 Troubleshoot issues**
→ [Getting Started - Troubleshooting](getting-started.md#troubleshooting)

## 🏗️ What is Vista SDK?

The Vista SDK provides Python tools for working with:

- **🚢 VIS (Vessel Information Structure)** - DNV's standardized data model for vessel equipment and systems
- **🏷️ Local IDs** - Unique, standardized identifiers for data channels following ISO 19847/19848
- **🌳 GMOD** - General Model of Data representing hierarchical equipment structure
- **📚 Codebooks** - Standardized vocabularies for metadata tags
- **📍 Locations** - Physical positioning information
- **🔄 Version Management** - Support for multiple VIS versions with conversion capabilities

## ⚡ Quick Example

```python
from vista_sdk import VIS, VisVersion, LocalIdBuilder, GmodPath

# Initialize and load VIS data
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)

# Create a standardized identifier for engine temperature
engine_path = GmodPath.parse("411.1/C101.31-2", VisVersion.v3_4a)
local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(engine_path)
    .with_quantity_tag("temperature")
    .with_position_tag("inlet")
    .build())

print(f"Local ID: {local_id}")
# Output: /vis-3-4a/411.1/C101.31-2/meta:qty.temperature-pos.inlet
```

## 🗂️ Documentation Structure

```
docs/
├── README.md           # This overview (you are here)
├── getting-started.md  # Quick start guide for newcomers
├── tutorials.md        # Step-by-step learning tutorials
└── API.md             # Complete technical reference
```

## 🤝 Contributing to Documentation

Found an error or want to improve the documentation? We welcome contributions!

### Quick Fixes
- **Typos/small errors**: Submit a pull request with the fix
- **Missing examples**: Add practical examples to help other users
- **Unclear explanations**: Suggest clearer wording or additional context

### Larger Contributions
- **New tutorials**: Add tutorials for specific use cases or workflows
- **Additional examples**: Create comprehensive examples for complex scenarios
- **Translation**: Help make documentation accessible in other languages

### Documentation Guidelines
- **Clear and concise**: Write for users with varying experience levels
- **Code examples**: Include working, tested code examples
- **Consistent formatting**: Follow the existing Markdown style
- **User-focused**: Write from the user's perspective, not the implementation

## 🔗 Additional Resources

### Official Documentation
- [DNV VIS Documentation](https://www.dnv.com/services/vessel-information-structure-vis-120226)
- [ISO 19847 Standard](https://www.iso.org/standard/66356.html)
- [ISO 19848 Standard](https://www.iso.org/standard/66357.html)

### Vista SDK Implementations
- [C# Implementation](../../csharp/) - Full-featured C# SDK
- [JavaScript Implementation](../../js/) - TypeScript/JavaScript SDK
- [Python Implementation](../) - This Python SDK

### Community
- [GitHub Repository](https://github.com/dnv-opensource/vista-sdk)
- [Issues & Bug Reports](https://github.com/dnv-opensource/vista-sdk/issues)
- [Discussions](https://github.com/dnv-opensource/vista-sdk/discussions)

## 📞 Support

### Self-Help Resources
1. **Search the documentation** - Use Ctrl+F to find specific topics
2. **Check examples** - Look at [samples/](../samples/) for working code
3. **Read error messages** - Most errors include helpful guidance
4. **Review troubleshooting** - Common issues are documented

### Getting Help
1. **GitHub Issues** - Report bugs or request features
2. **GitHub Discussions** - Ask questions and share experiences
3. **Stack Overflow** - Tag questions with `vista-sdk` and `python`

### Before Asking for Help
- [ ] Searched existing documentation
- [ ] Tried the examples in [samples/](../samples/)
- [ ] Checked for similar issues on GitHub
- [ ] Included code examples and error messages in your question

---

**Ready to get started?** 🚀 Head to the [Getting Started Guide](getting-started.md) to begin your Vista SDK journey!
