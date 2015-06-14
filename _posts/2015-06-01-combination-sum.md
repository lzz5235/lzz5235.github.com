---
layout: post
title: "Combination Sum 思路"
categories: others
tags: Algorithm
---
Combination Sum 思路
===================
最新刷leetcode的题目，发现了Combination Sum题目，这个题目分为I、II、III。难度层层递进，题目就是遍历vector容器，选择出符合target number的sum组合，这个题目的思路可以参考DFS通用解法。我们使用那个DFS模板，首先要构造dfs函数。

一般情况下，我们需要五个参数：结果，原始数据集，中间结果，当前指向的数据，满足target number的值：

然后我们根据candidates中的数据集，深搜这个数据集的各种可能性，将达成target的path中间结果加入result，按照通用模版

<pre><code>
void dfs(type &input, type &path, type &result, int cur or gap) {
              if (数据非法) return 0; // 终止条件
              if (cur == input.size()) { // 收敛条件
                  // if (gap == 0) {
                        将path 放入result
              }
              if (可以剪枝) return;
              for(...) { // 执行所有可能的扩展动作
                     执行动作，修改path
                     dfs(input, step + 1 or gap--, result);
                     恢复path
              }
}
</code></pre>

第一步：收敛也就是target==0

第二步：使用for(),并在循环中剪枝 if(target-candidates[current]<0) return;

第三步：如果通过第二部，也就意味着这个current合格，可以将这个加入到path中，然后继续深度遍历。dfs(result,candidates,path,current,target-candidates[current]);这里的问题是candidates的数据是可重复的，可以多次使用。如果只使用一次的话，也就意味着我们需要sort(),然后需要将上个满足条件的值跳过，也就是II中的nums[i]==nums[i-1]比较（Combination Sum II）

<pre><code>
void dfs(vector < vector< int> > &result,vector<int>& candidates,vector<int> path,int current,int target){
        if(!path.empty()&&target==0){
            result.push_back(path);
            return;
        }
        if(current<candidates.size()){
            int tmp = -1;//start from 0 and 1
            for(;current<candidates.size();current++){
                if(candidates[current]==tmp)
                    continue;
                if(target-candidates[current]<0)
                    return;
 
                tmp = candidates[current];
                path.push_back(candidates[current]);
                dfs(result,candidates,path,current+1,target-candidates[current]);
                path.pop_back();
            }
        }
    }
</code></pre>

 

##题目：

 

[https://leetcode.com/problems/combination-sum/](https://leetcode.com/problems/combination-sum/)

[https://leetcode.com/problems/combination-sum/](https://leetcode.com/problems/combination-sum/)

[https://leetcode.com/problems/combination-sum-iii/](https://leetcode.com/problems/combination-sum-iii/)
