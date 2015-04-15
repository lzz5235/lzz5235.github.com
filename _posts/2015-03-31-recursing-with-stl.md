Recursing with STL
==================
最近使用装有节点值的vector容器的前序遍历与中序遍历去构造一颗Binary Tree。

我们都知道先序遍历的顺序是 ：中-左-右，中序遍历的顺序是：左-中-右，后续遍历是左-右-中。

举例:

<pre><code>
//         1
//        / \
//       2   5
//      / \   \
//     3   4   6
</code></pre>

这棵树的先序遍历：1 2 3 4 5 6 中序遍历： 3 2 4 1 5 6 后续：3 4 2 6 5 1 。

众所周知，已知一个树的先序和中序，或者是中序和后续，便可以构造出一棵树，构造的思路是先序的第一个节点就是根，然后查找这个根在中序中的位置，中序遍历中根的位置的左边就是左子树，后面是右子树。然后递归进入到下一层：先序 2 3 4 中序 3 2 4 ，可以看出先序中2是根，然后查找中序2的位置，就得到 3 是 2 的左子树，4是2的右子树，依次recurse。

下面使用STL的方式构造这棵树：已知vector &preorder, vector &inorder是装有先序和中序的vector容器，使用递归就是设置出一个中间状态，对他进行分析，并且函数的参数必须是每次都可以用减小范围的。

###STL中，我们都知道有vector::iterator方式来遍历vector元素值，获取这种iterator有两种方式：1）preorder.begin()获取vector中第一个元素的指针。2）begin(preorder)，返回的也是vector第一个指针。他们都可以使用*得到值。

###end()比较特殊，它返回的是最后元素的下一个元素，也就是一个越界的元素，直接引用会导致数组越界。
auto pos = find(preorder.begin(),preorder.end(),val) 查找当前val的的节点，返回当前val的指针。
distance(preorder.begin(),pos) 直接返回中开始到pos的距离，不包括pos。

<pre><code>
//1 2 3 4 5 6
auto Pos = find(pre_order.begin(),pre_order.end(),4);
auto dis = distance(pre_order.begin(),Pos);
cout << dis << "\n";
cout << *next(pre_order.begin(),dis);
</code></pre>

我们发现dis = 3，而next是从1开始的3个元素后的下一个元素，也就是val=4的指针。

我们在定义这种迭代器声明的时候，会发现函数参数非常长

<pre><code>
TreeNode *buildTree(vector<int>::iterator inorder_first,vector<int>::iterator inorder_end,
        vector<int>::iterator postorder_first,vector<int>::iterator postorder_end)
</code></pre>

我们可以使用模板编程极大地简化声明：

<pre><code>
template<typename InputIterator>
TreeNode *buildTree(InputIterator inorder_first,InputIterator inorder_end,
        InputIterator postorder_first,InputIterator postorder_end)
{
    if(postorder_first==postorder_end)
        return NULL;
    if(inorder_first==inorder_end)
        return NULL;
    int val = *prev(postorder_end);
    TreeNode *root = new TreeNode(val);
 
    auto in_rootPos = find(inorder_first,inorder_end,val);
    auto left_size = distance(inorder_first,in_rootPos);
 
    root->left = buildTree(inorder_first,next(inorder_first,left_size),postorder_first,next(postorder_first,left_size));
    root->right = buildTree(next(inorder_first,left_size+1),inorder_end,next(postorder_first,left_size),prev(postorder_end));
 
    return root;
}
</code></pre>

上面的代码是后序遍历与中序遍历组合成一棵树，先序与中序也是类似的。

 

 

参考：

[1][http://www.cplusplus.com/reference/iterator/begin/?kw=begin](http://www.cplusplus.com/reference/iterator/begin/?kw=begin)

[2][http://www.cplusplus.com/reference/iterator/end/?kw=end](http://www.cplusplus.com/reference/iterator/end/?kw=end)

[3][http://www.cplusplus.com/reference/iterator/InputIterator/](http://www.cplusplus.com/reference/iterator/InputIterator/)

