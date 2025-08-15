# BuriedPoint 分支管理策略

## 🌳 分支结构

### 📦 **master 分支** - 生产就绪版本
- **目的**: 保持简洁、专业的项目展示
- **内容**: 
  - ✅ 核心源代码
  - ✅ 基本配置文件 (CMakeLists.txt, .gitignore)
  - ✅ 开源许可证 (LICENSE)
  - ✅ **唯一文档**: README.md
- **特点**: 
  - 🎯 面向最终用户和快速评估
  - 📦 生产级别的代码质量
  - 🚀 快速上手和集成

### 🔧 **dev 分支** - 开发和详细文档
- **目的**: 提供完整的开发资源和技术细节
- **内容**:
  - ✅ 所有 master 分支的内容
  - ✅ **docs/ 目录**: 详细技术文档
  - ✅ 开发过程中的分析和总结
  - ✅ 技术实现的深度解析
- **特点**:
  - 🔍 面向开发者和贡献者
  - 📚 完整的技术文档体系
  - 🛠️ 开发过程的完整记录

## 📁 文档分布策略

### Master 分支文档
```
BuriedPoint/
├── README.md          # 🎯 唯一文档：项目介绍、快速开始、API使用
└── ...                # 其他核心文件
```

### Dev 分支文档
```
BuriedPoint/
├── README.md          # 🎯 项目主文档
├── docs/              # 📚 详细技术文档目录
│   ├── README.md                           # 文档导航
│   ├── DYNAMIC_CONFIG_IMPLEMENTATION.md    # 动态配置实现分析
│   ├── THREAD_SAFETY_ANALYSIS.md          # 线程安全机制分析
│   ├── BRANCH_STRATEGY.md                 # 分支管理策略 (本文档)
│   └── ...                                # 未来的技术文档
└── ...                # 其他核心文件
```

## 🔄 工作流程

### 1. **功能开发**
```bash
# 在 dev 分支开发新功能
git checkout dev
# ... 开发代码 ...
# ... 创建技术文档 (存放在 docs/) ...
git commit -m "feat: 新功能实现"
```

### 2. **稳定版本发布**
```bash
# 将稳定功能合并到 master
git checkout master
git merge dev
# 确保 master 只有 README.md 文档
git push origin master
```

### 3. **文档管理**
```bash
# 详细技术文档只在 dev 分支维护
git checkout dev
# 在 docs/ 目录中添加/更新技术文档
git add docs/
git commit -m "docs: 更新技术文档"
git push origin dev
```

## 🎯 使用指南

### 对于最终用户
- 👉 **访问 master 分支**
- 📖 **阅读 README.md** 了解项目和使用方法
- 🚀 **快速开始** 集成和使用

### 对于开发者/贡献者
- 👉 **访问 dev 分支**
- 📚 **查看 docs/ 目录** 了解技术实现细节
- 🔧 **参与开发** 和文档完善

### 对于技术评估
- 👉 **master 分支**: 快速了解项目能力
- 👉 **dev 分支 docs/**: 深入了解技术实现

## ✅ 优势

1. **🎯 用户友好**: master 分支简洁明了，降低学习成本
2. **🔧 开发友好**: dev 分支提供完整的技术资源
3. **📦 专业形象**: 避免项目根目录文档泛滥
4. **🚀 快速评估**: 用户可以快速判断项目是否符合需求
5. **📚 深度学习**: 开发者可以深入了解技术细节

---

**维护原则**: 保持 master 分支的简洁性，将所有详细的技术文档集中在 dev 分支的 docs/ 目录中。