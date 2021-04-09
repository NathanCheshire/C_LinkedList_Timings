# timings-on-linkedlists
Performance comparison of thread-safe implementations of linked lists report

Homework 3 proved challenging and annoying to debug due to numerous segmentation faults with the hand-over-hand implementation of a locked list. For the list level lock implementation, writing the insert and lookup functions was pretty simple as the functions can lock the entire list until they are done. The hand-over-hand implementation, however, was a bit trickier to get working properly while avoiding race conditions and segmentation faults. For the regular list level locking list, I referenced our textbook as a starting point as it has some promising code that I needed to make sure worked properly.

The insert function for the hand-over-hand list always inserts at the head. When creating a hand-over-hand list, the initialize function sets a dummy node as the head with a next pointer of null. This is also how the lookup functions know when they are at the end of the list; if they reach a null pointer. Now that the list has a dummy node as a head, we will keep the dummy node as the head and insert after it whenever insert is called. The head’s next will be set to the new node we are inserting and this new node’s next is set to our head’s old next pointer. Thus, the insert function has a runtime complexity of O(1); constant time.

The lookup function for the hand-over-hand implementation will first lock the next node if it exists before releasing the lock on the current node. This function fails to find a value if it reaches a null next pointer. As mentioned in the project description, we may reduce the number of nodes in our list if it was taking a while to run. Thus, here are the results for 50K nodes and 100K nodes. This number may be adjusted in the code via a single variable above the main method.

![](https://i.imgur.com/vul0BU9.png)
![](https://i.imgur.com/c2HGeRm.png)
  
As seen from these times, the overhead caused by the hand-over-hand locking from having to wait for a lock on each node along with other factors that I’m sure exist but I am not aware of nor would understand result in the regular list to be faster than the hand-over-hand linked list in this situation. I reduced the number of nodes in the lists because it was taking over an hour to run the last task of looking up two million entries in the hand-over-hand list when starting with one million entries. I also believe it is a good point to make that I am running a dual boot of Ubuntu and not using a VM. I am thus not able to assign virtual cores to the program but since Ubuntu has access to the full power of my PC, 24 gigs of RAM, it is still significantly faster than running the code on a VM.


References:

http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/Book/threads-locks-usage.pdf - book chapter helped in understanding the concept of this homework and some of the code for the list level lock implementation
