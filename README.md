# formula_tree 属性公式计算树
游戏之中关于角色属性计算公式的编辑器与运行时
1. 编辑器 editor目录
2. 运行时 runtime目录
## 属性计算
在游戏之中，我们操纵的角色和一些非玩家角色都会有相关的数值描述，例如血量、等级、攻击、防御等等，下面就是暗黑破坏神3的一个角色的属性面板。

![暗黑破坏神3属性面板](http://i1.sinaimg.cn/gm/cr/2012/1123/1980164735.jpg)

事实上，一个角色的属性描述的几十个字段并不是毫无逻辑关系的，同时完整的属性关系之间可能还有其他的面板不可见属性作为中间变量存在。所有可见不可见的变量会组成一个有向无环图。图中的每个节点都有一个对应的值，当这个值更新之后，会触发当前节点可及的节点的更新，并递归执行更新过程。更新过程完成之后，通知外部模块进行响应，例如刷新血量、刷新面板等操作。

下面的就是一个典型的`dota`计算公式：
> 护甲 = 基础护甲 + 额外护甲 + 敏捷 / 3

而敏捷这个变量其实也是由计算公式生成的
> 敏捷 = (基础敏捷 + 等级 * 敏捷成长) *（1 + 敏捷放大倍数） + 额外敏捷

这里可以看出，等级提升之后，敏捷会相应提升，并因此更新护甲。
角色的属性系统除了让面板变得更好看之外，最重要的作用是计算角色之间的伤害，一次攻击，附加伤害是多少，是否会暴击，准确率是多少，是否被会闪避。
> 我这一刀下去你可能会死

这种属性更新计算在大型的pvp活动中调用非常频繁，需要一个高效的结构去维护更新。同时由于技能和buff、道具系统的的无限扩充，整个属性图中可能有上千个节点，人工维护公式逻辑已经不太现实，需要提供更明了的工具来编辑公式及转换为代码。当前项目就是因此而生：

## 编辑器
属性编辑器 直接复用了我之前的那个树形编辑器，没有其他的依赖：
1. tree_editor https://github.com/huangfeidian/tree_editor

![护甲计算公式](https://github.com/huangfeidian/md_for_blog/raw/master/images/formula/armor.png)

![敏捷计算公式](https://github.com/huangfeidian/md_for_blog/raw/master/images/formula/dexterity.png)

这里说一下这个属性公式编辑器的工作流程：

1. 叶子节点有三种类型，字面值常量，输入变量(`input name`)，引用变量(`import name`)， 输入变量只能被外部修改，而引用变量则是通过计算公式计算出来的值，无法直接被外部修改，只能通过修改输入变量来更新到引用变量。每个引用变量都有对应名字的公式输出文件。
2. 提供`choice.json` 里面有两个字段 一个是`input_attrs`，这个是所有的输入节点的名字 一个是`import_attrs`，这个是所有的输出节点的名字。如果需要添加输入变量或者输出变量，则需要更新这个文件
3. 提供`formula_nodes.json` 里面有所有类型节点的编辑器相关字段，如果想添加计算函数，需要更新这个文件

## 运行时

运行时依赖的如下几个库：
1. nlohmann_json https://github.com/nlohmann/json 公式文件格式
2. magic_enum https://github.com/Neargye/magic_enum 处理枚举与字符串之间的转换
3. any_container https://github.com/huangfeidian/any_container 我自己的一个库 处理类型与json之间的转换

一个角色的所有属性所需公式被称为一组公式，里面有所有外部可见的输出变量名称。 
1. 公式系统会装载所有提供的公式，并对公式内部所引用的输出变量也进行递归加载。
2. 每个对输出变量的引用都会生成一条输出变量的root节点到当前import节点的边，通过这样的连接，组成了一个有向图。如果这个有向图里面有环的话，代表变量之间互相引用了，这是一个非法的公式。
3. 在组成一个有向无环图之后，我们在删除所有的引用节点，把从引用节点出发的边的起点都转移到对应的输出变量的root节点上。降低一点树的深度。


自此，一个角色的公式计算图构建完成，现在我们来描述一下更新逻辑。

简单版本的公式更新就是：每更新一个节点，就深度优先的更新他的可达节点。但是这样的更新有很严重的问题，如果从这个节点A出发到某个节点B有多条路径，则B节点及从B出发可达的节点会被重复更新多次。类似的问题在一次性更新多个变量的时候也存在，如果节点A依赖于输入节点B和输入节点C， 如果B、C的值都发生了改变，会导致A被更新多次。所以我们需要提供一个最优更新逻辑，保证一个节点最多只被更新一次。因此，在上面构建的公式计算图的基础上，我们需要标注节点的额外信息：
> 对每个节点进行高度标记，所有的输入节点的高度设置为0，然后进行递归更新，每个节点的高度等于所有子节点的高度最大值再加上1

在这个新增加的信息基础上， 我们提供了单变量更新和批量更新，其实单变量更新就是只有一个变量的批量更新，所以我们这里只阐述批量更新的逻辑：
1. 将所有的需要更新的输入变量放到一个任务队列中
2. 只要队列不为空，从队列中取出高度值最低的节点，进行更新计算，如果值进行了改变，则将当前节点的所有可达节点中不在任务队列中的节点加入到任务队列
3. 如此重复直到任务队列为空

在上面的更新结构下，我们就保证了一个节点最多被更新一次。

## 优化

这个项目是我花了清明两天在原来的行为树编辑器的基础上做出来的，还残缺了很多实用的功能：
1. 编辑器通过手动输入公式来自动添加子节点
2. 编辑器保存时检查是否有环 并警告
3. 运行时可以将逻辑和数据分离，每个角色只负责存储数据，整个公式结构由单例来管理，这样分配内存更简单。更新时只需要将数据的引用传递到公式即可


