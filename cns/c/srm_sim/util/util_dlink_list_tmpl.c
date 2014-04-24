#ifdef T


TNAME(T,LList)* TEMPLATE(createLList,T)() {
    TNAME(T,LList) *a = (TNAME(T,LList)*)malloc( sizeof(TNAME(T,LList)));
    a->size=0;
    a->first = NULL;
    a->last = NULL;
    a->current = NULL;
    return(a);
}
TNAME(T,LNode)* TEMPLATE(getFirstLList,T)(TNAME(T,LList) *a) {
    a->current = a->first;
    return(a->current);
}
TNAME(T,LNode)* TEMPLATE(getNextLList,T)(TNAME(T,LList) *a) {
    if(a->size == 0) return(NULL);
    if(a->current == NULL) {
        a->current = a->first;
    } else {
        a->current = a->current->next;
    }
    return(a->current);
}

TNAME(T,LNode)* TEMPLATE(getPrevLList,T)(TNAME(T,LList) *a) {
    if(a->size == 0) return(NULL);
    if(a->current == NULL) {
        a->current = a->last;
    } else {
        a->current = a->current->prev;
    }
    return(a->current);
}

void TEMPLATE(addValueLList,T)(TNAME(T,LList) *a, T value) {
    TNAME(T,LNode) *an = (TNAME(T,LNode)*) malloc(sizeof(TNAME(T,LNode)));
    if(!an) {
        printf("malloc failed\n");
        exit(1);
    }
    an->value = value;
    if(a->size == 0) {
        a->first = an;
        a->last = an;
        an->next = NULL;
        an->prev = NULL;
    } else {
        a->last->next = an;
        an->prev = a->last;
        a->last = an;
        a->last->next = NULL;
    }
    ++a->size;
}

void TEMPLATE(insertAfterLList,T)(TNAME(T,LList) *a, TNAME(T,LNode) *n, T value) {
    TNAME(T,LNode) *an = (TNAME(T,LNode)*) malloc(sizeof(TNAME(T,LNode)));
    if(!an) {
        printf("malloc failed\n");
        exit(1);
    }
    an->value = value;
    if(n == NULL) { //adding in first place
        if(a->size == 0) {
            a->first = an;
            a->last = an;
            an->next = NULL;
            an->prev = NULL;
        } else {
            a->first->prev= an;
            an->next = a->first;
            a->first = an;
            a->first->prev = NULL;
        }
    } else {
        if(n->next == NULL) { // adding in last place
            a->last->next = an;
            an->prev = a->last;
            a->last = an;
            a->last->next = NULL;
        } else {
            // adding in middle
            TNAME(T,LNode) *n_acc = n->next;
            n->next = an;
            an->prev = n;
            an->next = n_acc;
            n_acc->prev = an;
        }
    }
    ++a->size;
}

void TEMPLATE(dropNodeLList,T)(TNAME(T,LList) *a, TNAME(T,LNode) *a_curr) {
    if(a->size == 0) { 
        printf("Error in dropNode. Dropping in empty list\n");
        exit(1);
    }
    if(a_curr == NULL) {
        printf("Dropping NULL value\n");
        exit(1);
    }
    
    TNAME(T,LNode) *an = a_curr->next; // NULL or next
    TNAME(T,LNode) *ap = a_curr->prev; // NULL or prev
    if(ap) {
        if(an) {
            ap->next = an;
            an->prev = ap;
        } else {
            ap->next =NULL;
            a->last = ap;
        }
    } else {
        if(an) {
            an->prev = NULL;
            a->first = an;
        } else {
            a->first = NULL;
            a->last = NULL;
        }
    }
    a->current = ap;
    a->size--;
    if(a->size == 0) {
        a->first = NULL;
        a->last = NULL;
    }
    free(a_curr);
}

void TEMPLATE(deleteLList,T)(TNAME(T,LList) *a) {
    TNAME(T,LNode) *an = TEMPLATE(getFirstLList,T)(a);
    while( an != NULL ) {
        TNAME(T,LNode) *next = an->next;
        free(an);
        an = next;
    }
    a->first=NULL;
    a->last=NULL;
    a->current=NULL;
    free(a);
}

#endif
