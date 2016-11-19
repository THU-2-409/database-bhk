<link href="/home/fool/Public/mdCSS/avenir-white-my.css" rel="stylesheet" />

# database-bhk

数据库大作业



## 设计

*字符串存储：* **以\0结尾**

### TableHeader.h

应该不需要改

### Record.h

| 接口 | 功能 | 备注 |
| --- | --- | --- |
| `pair<bool, int> getInt(string)` | | |
| `pair<bool, string> getString(string)` | | |
| `bool setInt(string, int, bool)` | | 布尔参数false时，代表null值 |
| `bool setString(string, string, bool)` | | |

### Table.h

| 接口 | 功能 | 备注 |
| --- | --- | --- |
| `static pair<bool, Table> createFile(TableHeader header, string path)` | 创建数据表文件 | 记录管理 |
|`static int deleteFile(string path)`|删除文件||
|`bool open(string path)`|打开文件||
|`int close()`|关闭文件||
|`Record insertRecord(Record)`|插入||
|`bool deleteRecord(Record)`|删除||
|`bool updateRecord(Record real, Record dummy)`|更新||
|`pair<bool,Record> selectRecord(Record cond)`|查询||
|`void createIndex(string name)`|为属性创建索引|索引模块|
|`void deleteIndex(string name)`|删除属性索引|索引模块|
|`int openIndex(string name)`|打开索引，返回索引的根页|索引模块|
|`void closeIndex(string name)`|关闭索引缓存|索引模块|
|`void insertIntoIndex(int page, Record record)`|插入记录至索引|索引模块|
|`void deleteFromIndex(int page, Record record)`|删除索引中记录|索引模块|
|`Record findFromIndex(int page, Record record)`|查询索引中记录|索引模块|

| 私有方便接口 | 功能 | 备注 |
| --- | --- | --- |
| `char* getChars(int page, int offset, int size)`|返回读数据指针||
| `bool setChars(int page, int offset, char* buf, int size)`|写入数据||
| `int getInt(int page, int offset)`| 读整数| |
| `string getString(int page, int offset)`| 读字符串| |

#### 第一页组织

1. 表头
    1. 属性个数
    2. 每个属性在一个记录中占的存储空间
    3. 每个属性的名字
    4. 每个属性的约束
        1. 约束个数
        2. 每个约束
            1. 约束类型
            2. 相关值
2. 数据页链表头
3. 记录空位链表头
4. m个（索引页序号（B树根），第几个属性）
5. 空页链表头
6. [var数据页链表头]

#### 数据页组织

页头next指针

#### 记录组织

开始是null位数组，之后依次存储每个属性

#### 索引页组织

- 普通的非叶结点
    - <值,指针>键对。
    - 终结位的指针为-1
- 叶结点
    - 记录的拷贝
    - 页尾offset倒排数组：本页记录按索引属性排序
    - offset以short存储
    - 数组哨兵：-1
    - 存储量公式(字节)：(页大小-2)/(记录大小+2)

#### [var数据页]

页头next指针























<!--
第一页：
总长度 int
表项数量 n
n 个 int 每个表项的长度 负号 可变（可变长度的字段在另外的页）
n个 （表项名称长度byte，名称）
记录总数
记录页数
（每一页的记录个数， 每一页的第一个空位）

每一页
从页末尾倒序位数组 0 未使用 1 已使用


!!! NULL值 !!!
未实现，每个记录需要一个bit数组来记录NULL
-->

