#include <cstdint>
#include <algorithm>
#include <map>

#include "database.hpp"

template<typename T>
struct minSet {
    struct item {
        GraphNode key;
        double prior;
        T value;
        item *l, *r;

        item() : key(0), prior(0), l(nullptr), r(nullptr) {};

        item(GraphNode key, double prior, const T &value) : key(key), prior(prior), value(value), l(nullptr), r(nullptr) {};
    };

    typedef item *pitem;
    pitem root;

    minSet() : root(nullptr) {};

    void recurDel(pitem t) {
        if (t->l) recurDel(t->l);
        if (t->r) recurDel(t->r);
        delete (t);
    }

    ~minSet() {
        if (root) recurDel(root);
    }

    void _split(pitem t, GraphNode key, pitem &l, pitem &r) {
        if (!t)
            l = r = nullptr;
        else if (key < t->key)
            _split(t->l, key, l, t->l), r = t;
        else
            _split(t->r, key, t->r, r), l = t;
    }

    void _insert(pitem &t, pitem it) {
        if (!t)
            t = it;
        else if (it->prior < t->prior)
            _split(t, it->key, it->l, it->r), t = it;
        else
            _insert(it->key < t->key ? t->l : t->r, it);
    }

    void _merge(pitem &t, pitem l, pitem r) {
        pitem new_t = nullptr;
        if (!l || !r)
            new_t = l ? l : r;
        else if (l->prior < r->prior)
            _merge(l->r, l->r, r), new_t = l;
        else
            _merge(r->l, l, r->l), new_t = r;
        t = new_t;
    }

    void _erase(pitem &t, GraphNode key) {
        if (!t) {
            return;
        }
        if (t->key == key) {
            pitem to_del = t;
            _merge(t, t->l, t->r);
            delete (to_del);
        } else
            _erase(key < t->key ? t->l : t->r, key);
    }

    pitem _unite(pitem l, pitem r) {
        if (!l || !r) return l ? l : r;
        if (l->prior > r->prior) std::swap(l, r);
        pitem lt, rt;
        split(r, l->key, lt, rt);
        l->l = unite(l->l, lt);
        l->r = unite(l->r, rt);
        return l;
    }

    pitem _search(GraphNode key) {
        pitem p = root;
        while (p && p->key != key) {
            p = (key < p->key ? p->l : p->r);
        }
        return p;

    }

    item getmin() {
        if (root) {
            return *root;
        }
        return item();
    }

    item get(GraphNode key) {
        pitem p = _search(key);
        if (p) {
            return *p;
        } else {
            return item();
        }
    }

    void erase(GraphNode key) {
        _erase(root, key);
    }

    bool is_empty() {
        return !root;
    }

    void update(GraphNode key, double prior, const T &value) {
        pitem it = _search(key);
        if (it) {
            erase(key);
        }
        it = new item(key, prior, value);
        _insert(root, it);
    }
};

/*
struct minimumsSet {
	struct minimumsSetElement {
		double key;
		double value;
		uint64_t edgeIndex;
		uint64_t nodeIndex;
		char inf;
	};
	std::vector<minimumsSetElement> _array;
	std::vector<char> used;
	bool is_empty() {
		bool ans = true;
		for (auto& e : used) {
			if (e == 0) {
				ans = false;
			}
		}
		return ans;
	}
	minimumsSetElement getMinimum() {
		minimumsSetElement ans = {
			0, 0, 0, 0, 1
		};
		for (uint64_t i = 0; i < _array.size(); i++) {
			if (used[i] == 0 &&(ans.inf || ans.key > _array[i].key)) {
				ans = _array[i];
			}
		}
		return ans;
	}

	minimumsSetElement get(uint64_t nodeIndex) {
		bool updated = false;
		minimumsSetElement ans = {
			0, 0, 0, 0, 1
		};
		for (uint64_t i = 0; i < _array.size(); i++) {
			if (used[i] == 0 &&nodeIndex == _array[i].nodeIndex) {
				ans = _array[i];
			}
		}
		return ans;
	}
	void update(uint64_t nodeIndex, double weight,  uint64_t edgeIndex, double shading) {
		bool updated = false;
		minimumsSetElement el = {
			 weight, shading, edgeIndex, nodeIndex
		};
		for (uint64_t i = 0; i < _array.size(); i++) {
			if (nodeIndex == _array[i].nodeIndex) {
				_array[i] = el;
				updated = true;
				used[i] = 0;
			}
		}
		if (!updated) {
			_array.push_back(el);
			used.push_back(0);
		}
	}

	void erase(uint64_t nodeIndex) {
		for (uint64_t i = 0; i < _array.size(); i++) {
			if (nodeIndex == _array[i].nodeIndex) {
				used[i] = 1;
			}
		}
	}
};
*/

struct minimumsSet {
    struct minimumsSetElement {
        double key;//Приоритет
        double value;
        GraphNode edgeIndex;
        GraphNode nodeIndex;//Ключ
        char inf;
    };

    struct minSetType {
        double value;
        GraphNode edgeIndex;
        char inf;

        minSetType() : value(0), edgeIndex(0), inf(1) {}

        minSetType(double value, GraphNode edgeIndex = GraphNode(0), char inf = 1) : value(value), edgeIndex(edgeIndex),
                                                                         inf(inf) {};
    };

    minSet<minSetType> treap;

    bool is_empty() {
        return treap.is_empty();
    }

    minimumsSetElement getMinimum() {
        auto ret = treap.getmin();
        minimumsSetElement ans = {
                ret.prior,
                ret.value.value,
                ret.value.edgeIndex,
                ret.key,
                ret.value.inf
        };
        return ans;
    }

    minimumsSetElement get(GraphNode nodeIndex) {
        auto ret = treap.get(nodeIndex);
        minimumsSetElement ans = {
                ret.prior,
                ret.value.value,
                ret.value.edgeIndex,
                ret.key,
                ret.value.inf
        };
        return ans;
    }

    void update(GraphNode nodeIndex, double weight, GraphNode edgeIndex, double shading) {
        bool updated = false;
        minSetType el = {
                shading, edgeIndex, 0
        };
        treap.update(nodeIndex, weight, el);
    }

    void erase(GraphNode nodeIndex) {
        treap.erase(nodeIndex);
    }
};

struct usedSet {
    std::map<GraphNode, bool> _map;

    void update(GraphNode nodeIndex, char used) {
        _map[nodeIndex] = used;
    }

    bool get(GraphNode nodeIndex) {
        return _map[nodeIndex];
    }
};

struct valueSet {
    struct valueSetElement {
        GraphNode nodeIndex;
        double value;
        GraphNode edgeIndex;
        GraphNode prevNodeIndex;
    };
    std::map<GraphNode, valueSetElement> _map;

    void update(GraphNode nodeIndex, double value, GraphNode edgeIndex, GraphNode prevNodeIndex) {
        valueSetElement el = {
                nodeIndex, value, edgeIndex, prevNodeIndex
        };
        _map[nodeIndex] = el;
    }

    valueSetElement get(GraphNode nodeIndex) {
        return _map[nodeIndex];
    }
};

static std::vector<double> trans_finesness = {
        1, //very low priority      0
        5, //low priority           1
        10, //lower priority        2
        100, //common road          3
        500, //higher priority      4
        1000, //high priority       5
        5000, //very high priority   6
        50, //common road          7 l-c priority
};
