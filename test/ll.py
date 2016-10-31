class Node(object):
    def __init__(self, val):
        self.val = val
        self.next = None


class LinkedList(object):
    def __init__(self):
        self.head = None


    def push_back(self, val):
        if self.head is None:
            self.head = Node(val)
            return
        curr = self.head

        while not curr.next is None:
            curr = curr.next
        
        curr.next = Node(val)

        
    def delete_node(self, id):
        assert not self.head is None, "Trying to delete element from empty list"
        if id == 0:
            self.head = self.head.next if self.head.next else None
            return

        curr_id = 0
        curr = self.head
                    
        while curr_id < id:
            assert not curr.next is None, "Out of list size"
            if curr_id == id - 1:
                break
            curr = curr.next
            curr_id += 1

        next_after_deleted = curr.next.next 
        curr.next = None
                
        if not next_after_deleted is None:
            curr.next = next_after_deleted

    def reverse_list(self):
    	if self.head is None or self.head.next is None:
    		return
    	
    	prv, cur, nxt = None, self.head, self.head.next
    	
    	while not nxt is None:
    		nxt = cur.next
    		cur.next = prv
    		prv, cur = cur, nxt
    	
    	self.head = prv



    def __str__(self):
        curr = self.head
        debug_string = "["
        while not curr is None:
            debug_string += "{} -> ".format(curr.val) 
            curr = curr.next
        debug_string += "]"
        return debug_string




        
        
ll = LinkedList()

for i in xrange(10):
    ll.push_back(i)
    
print ll

ll.reverse_list()

print ll