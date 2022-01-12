#ifndef PF_BUFFER_STRATEGY_HH
#define PF_BUFFER_STRATEGY_HH

#include <unordered_map>

template <typename T>
class BufferStrategy {
public:
    virtual ~BufferStrategy() = default;
    virtual void visit(T &t) = 0;
    virtual int size() const = 0;
    virtual bool empty() const = 0;
    virtual bool contain(T &t) const = 0;
    virtual T pop() = 0;
    virtual void remove(T &t) = 0;
    virtual void push(T &t) = 0;
};

/*
 * least recent used.
 * 优先淘汰最长时间没有使用的元素
 */
template <typename T, typename hash>
class LRU : public BufferStrategy<T> {
private:
    struct ListNode {
        T t;
        ListNode *next, *pre;
    };

public:
    LRU();
    ~LRU();
    void visit(T &t) override;
    inline int size() const override;
    inline bool empty() const override;
    inline bool contain(T &t) const override;
    T pop() override;
    void remove(T &t) override;
    void push(T &t) override;

private:
    void addToListHead(ListNode* node);

    void removeNodeFromList(ListNode* node);

private:
    ListNode* head_;
    ListNode* tail_;
    std::unordered_map<T, ListNode*, hash> eleToNode_;
};


template <typename T, typename hash>
LRU<T, hash>::LRU()
    : head_(new ListNode)
    , tail_(new ListNode)
{
    head_->next = tail_;
    tail_->pre = head_;
}

template <typename T, typename hash>
LRU<T, hash>::~LRU()
{
    ListNode* dummy = head_;
    while (dummy != nullptr) {
        ListNode* tmp = dummy;
        dummy = dummy->next;
        delete tmp;
    }
}

template <typename T, typename hash>
void LRU<T, hash>::visit(T &t)
{
    ListNode* node = eleToNode_[t];
    removeNodeFromList(node);
    addToListHead(node);
}

template <typename T, typename hash>
int LRU<T, hash>::size() const
{
    return eleToNode_.size();
}

template <typename T, typename hash>
bool LRU<T, hash>::empty() const
{
    return size() == 0;
}

template <typename T, typename hash>
bool LRU<T, hash>::contain(T &t) const
{
    return eleToNode_.find(t) != eleToNode_.end();
}

template <typename T, typename hash>
void LRU<T, hash>::push(T &t)
{
    ListNode* node = new ListNode { t };
    addToListHead(node);
    eleToNode_[t] = node;
}

template <typename T, typename hash>
T LRU<T, hash>::pop()
{
    ListNode* node = tail_->pre;
    removeNodeFromList(node);
    T rs = node->t;
    eleToNode_.erase(rs);
    delete node;
    return rs;
}

template <typename T, typename hash>
void LRU<T, hash>::remove(T &t)
{
    ListNode* node = eleToNode_[t];
    removeNodeFromList(node);
    eleToNode_.erase(t);
    delete node; 
}

template <typename T, typename hash>
void LRU<T, hash>::addToListHead(ListNode* node)
{
    node->next = head_->next;
    head_->next->pre = node;
    node->pre = head_;
    head_->next = node;
}

template <typename T, typename hash>
void LRU<T, hash>::removeNodeFromList(ListNode* node)
{
    node->pre->next = node->next;
    node->next->pre = node->pre;
    node->pre = node->next = nullptr;
}

#endif