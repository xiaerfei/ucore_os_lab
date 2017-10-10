## Parse


[ucore-QA](https://chyyuu.gitbooks.io/os_course_qa/content/02-interrupt-exception-syscall.html)

[GDT 与 LDT](http://www.cnblogs.com/hicjiajia/archive/2012/05/25/2518684.html)

![gdtr](./Source/gdtr.png)

> P : present位

* 1表示所描述的段存在(有效)
* 0表示所描述的段无效,使用该描述符会引起异常 

> DPL : Descriptor privilege

描述符特权级别,说明所描述段的特权级别 

> DT

描述符类型位,1说明当前描述符为存储段描述符,0为系统描述符或门描述符. 

> TYPE: 

* 位0:A(accessed)位,表明描述符是否已被访问;把选择子装入段寄存器时,该位被标记为1 
* 位3:E(EXECUTABLE?)位,0说明所描述段为数据段;1为可执行段(代码段) 

当为数据段时:

   * 位1为W位,说明该数据段是否可写(0只读,1可写) 
   * 位2为ED位,说明该段的扩展方向(0向高位扩展,1向低位扩展)

当为可执行段:

   * 位1为R位,说明该执行段是否可读(0只执行,1可读) 
   * 位2为C位,0说明该段不是一致码段(普通代码段),1为一致码段 

> G:
  
  粒度位,0说明LIMIT粒度为字节,1为4K字节. 

> D位: 
   * 1.在可执行段中,D为1,表示使用32位地址,32/8位操作数;为0表示使用16位地址,16/8位操作数 
   * 2.在由SS寻址的段描述符(堆栈段?)中,D为1表示隐含操作(如PUSH/POP)使用ESP为堆栈指针,/
     为0使用SP(隐含操作:未明确定义段属性类型USE16/USE32?66H,67H?) 
   * 3.在向低扩展的存储段中,D为1,表示段的上限为4G;为0上限为64K