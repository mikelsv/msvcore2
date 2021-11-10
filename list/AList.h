// Auto List
// 11.11.2015 12:50

enum AListOpt{ AListCon = 1, AListZero = 3, AListDes = 2, AListClear = 4 };

// AListAllocDef - Malloc / Free
// AListAllocUList - Linked buffers. Free all memory in Clear().
// AListAllocOList - Linked buffers. Free block if don't use.
// AListAllocOListFast - Linked buffers. Free block if don't use. Fast free, + sizeof(void*) memory per element.

// Malloc / Free
template<class AListEl, int AListOps = AListCon | AListDes>
class AListAllocDef{

public:
	AListAllocDef(){
	}

	AListEl* AllocNew(){
		AListEl *el = (AListEl*)malloc(sizeof(AListEl));

		if(AListOps & AListCon)
			new(el)AListEl;
		else if(AListOps & AListZero)
			memset(el, 0, sizeof(AListEl));

		return el;
	}

	void AllocFree(AListEl *el){
		if(!el)
			return ;

		if(AListOps & AListDes)
			el->~AListEl();
		free(el);
		return ;
	}

};


// Allocate block[AListEl * x]. No free(block), free in destroy.
// Warning: sizeof(AListEl) must be >= sizeof(void*);
template<class AListEl, int AListOps = AListCon | AListDes | AListClear, int blocksize = 16384>
class AListAllocUList{

	// Node structure
	class AListAllocUListNode{
	public:
		AListAllocUListNode *_n;
//		AListEl data[0];
	};

	// Free structure
	class AListAllocUListFel{
	public:
		AListAllocUListFel *el;
	};

	// Nodes
	AListAllocUListNode *_a, *_e;

	// Free elements
	AListAllocUListFel *fel;

public:
	AListAllocUList(){
		_a = 0;
		_e = 0;
		fel = 0;
	}

	AListEl* AllocNew(){
		AListEl *el;

		if(!fel)
			AllocNode();

		el = (AListEl*)fel;
		fel = fel->el;

		if(AListOps & AListCon)
			new(el)AListEl;
		else if(AListOps & AListZero)
			memset(el, 0, sizeof(AListEl));

		return el;
	}

	void AllocFree(AListEl *el){
		if(!el)
			return ;

		if(AListOps & AListDes)
			el->~AListEl();

		AListAllocUListFel *del = (AListAllocUListFel*)el;
		del->_n = fel;
		fel = del;
		
		return ;
	}

private:
	void AllocNode(){
		int els = (blocksize - sizeof(AListAllocUListNode)) / sizeof(AListEl);
		AListAllocUListNode *node = (AListAllocUListNode*) malloc(sizeof(AListAllocUListNode) + els * sizeof(AListEl));
		OMatrixTemplateAddO(_a, _e, node);

		// Add elements to free list
		AListEl *el = (AListEl*)(node + 1), *to = el + els;
		AListAllocUListFel *f;

		while(el < to){
			f = (AListAllocUListFel*) el;
			f->el = fel;
			fel = f;
			el ++;
		}

		return ;
	}

public:

	void Clean(){
		AListAllocUListNode *n = _a, *d;

		while(n){
			d = n;
			n = n->_n;
			free(d);
		}

		_a = 0;
		_e = 0;
		fel = 0;

		return ;
	}

	~AListAllocUList(){
		Clean();
	}

};


// Object Allocate
// Allocate: if(!free_element) Allocate node; return free_element;
// Allocate node: malloc(array of AListEl); => free_element;
// Free: hashtable -> node.used_size --; if(!node.used size) free(node);

// Free structure ///////
class AListAllocOListFel{
public:
	AListAllocOListFel *_n;
};

// Node structure ////////
class AListAllocOListNode{
public:
	// Left & right (linked)
	AListAllocOListNode *_p, *_n;

	// Prev & Next free (linked) node
	AListAllocOListNode *_pf, *_nf;

	// Free elements
	AListAllocOListFel *fel;

	// Used size
	unsigned int usz;

	// Data
	//AListEl data[0];

	void Clear(){
		_p = 0;
		_n = 0;
		_pf = 0;
		_nf = 0;
		fel = 0;
		usz = 0;
	}

	AListAllocOListNode(){
		Clear();
	}

};

// AListVoidVector
class AListVoidVector{

	// Pointer
	class NodePoi{
	public:
		void *node;
	};

	// Nodes
	NodePoi *nodes;

	// Nodes Size
	unsigned int nsz;

	// Nodes All
	unsigned int asz;

public:
	AListVoidVector(){
		nodes = 0;
		nsz = 0;
		asz = 0;
	}

	void Add(void *node){
		if(nsz >= asz)
			Resize();

		NodePoi *f = nodes, *to = nodes + nsz, *p = 0;
		int sz = nsz, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->node < node){
				f = ++p; 
				sz -= s + 1;
			}
			else
				sz = s;
		}

		if(f != (nodes + nsz))
			memcpy(f + 1, f, (nsz - (f - nodes)) * sizeof(NodePoi));
		nsz ++;

		f->node = node;

		return ;
	}

	void Resize(){
		if(nsz < asz)
			return ;

		int nasz = asz ? asz << 1 : 1024;
		NodePoi *nnodes = (NodePoi*) malloc(nasz * sizeof(NodePoi));

		memcpy(nnodes, nodes, nsz * sizeof(NodePoi));

		free(nodes);

		asz = nasz;
		nodes = nnodes;

		return ;
	}

	void* Get(void *pos){
		if(!nodes)
			return 0;

		NodePoi *f = nodes, *to = nodes + nsz, *p = 0;
		int sz = nsz, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->node < pos){
				f = ++ p; 
				sz -= s + 1; 
			}
			else
				sz = s;
		}

		if(f && f < nodes + nsz)
			return f->node;
		return 0;
	}

	void Del(void *node){
		if(!nodes)
			return ;

		NodePoi *f = nodes, *to = nodes + nsz, *p = 0;
		int sz = nsz, s;

		while(sz){
			p = f;
			s = sz / 2;
			p += s;

			if(p->node < node){
				f = ++p; 
				sz -= s + 1; 
			}
			else
				sz = s;
		}

		memcpy(f, f + 1, (nsz - (f - nodes) - 1) * sizeof(NodePoi));
		nsz --;
	}

	void Clean(){
		free(nodes);

		nodes = 0;
		nsz = 0;
		asz = 0;
	}

	~AListVoidVector(){
		Clean();
	}

};

template<class AListEl, int AListOps = AListCon | AListDes | AListClear, int blocksize = 16384>
class AListAllocOList{

	// Nodes
	AListAllocOListNode *_a, *_e;

	// Free nodes
	AListAllocOListNode *_af, *_ef;

	// Vector
	AListVoidVector vec;

	// Nodes Size
	unsigned int nsz;

	// Elements Size
	unsigned int esz;

public:
	AListAllocOList(){
		Init();
	}

	void Init(){
		_a = 0;
		_e = 0;
		_af = 0;
		_ef = 0;
		esz = 0;
		nsz = 0;
	}

	AListEl* AllocNew(){
		if(!_af)
			AllocNode();

		AListEl *el = (AListEl*) _af->fel;
		_af->fel = _af->fel->_n;
		_af->usz ++;		

		if(!_af->fel){
			if(_af->_nf)
				_af = _af->_nf;
			else
				_af = 0;
		}

		if(AListOps & AListCon)
			new(el)AListEl;
		else if(AListOps & AListZero)
			memset(el, 0, sizeof(AListEl));

		return el;
	}

	void AllocFree(AListEl *el){
		if(!el)
			return ;

		if(AListOps & AListDes)
			el->~AListEl();

		AListAllocOListNode *node;
		node = GetNode(el);

		//if(!node)
		//	int err = 546;

		AListAllocOListFel *fel = (AListAllocOListFel*)el;
		fel->_n = node->fel;
		node->fel = fel;
		node->usz --;

		if(!node->usz)
			FreeNode(node);

		return ;
	}

private:

	void AllocNode(){
		// New
		int els = (blocksize - sizeof(AListAllocOListNode)) / sizeof(AListEl);
		AListAllocOListNode *node = (AListAllocOListNode*) malloc(sizeof(AListAllocOListNode) + els * sizeof(AListEl));
		node->Clear();
		nsz ++;

		// Add to vector
		AListAllocOListNode *vnode = (AListAllocOListNode*) vec.Get(node);
		vec.Add(node);

		// Add to list
		if(!vnode){
			OMatrixTemplateAdd(_a, _e, node);
		}
		else{
			if(vnode > node)
				vnode = vnode->_p;
			OMatrixTemplateAddP(_a, _e, vnode, node);
		}
		
		// Add elements to free list
		AListEl *el = (AListEl*)(node + 1), *to = el + els;
		AListAllocOListFel *fel;

		while(el < to){
			fel = (AListAllocOListFel*) el;
			fel->_n = node->fel;
			node->fel = fel;
			el ++;
		}

		OMatrixTemplateAddF(_af, _ef, node, _pf, _nf);

		return ;
	}

	AListAllocOListNode* GetNode(AListEl *el){
		AListAllocOListNode *vnode = (AListAllocOListNode*) vec.Get(el);

		if(!vnode)
			vnode = _e;
		else
			vnode = vnode->_p;

		//int els = (blocksize - sizeof(AListAllocOListNode)) / sizeof(AListEl);
		//int sz = sizeof(AListAllocOListNode) + els * sizeof(AListEl);

		//if((char*)el < (char*)vnode || (char*)el > (char*)vnode + sz)
		//	int err = 567;

		return vnode;
	}

	void FreeNode(AListAllocOListNode *node){
		//return;

		OMatrixTemplateDel(_a, _e, node);
		OMatrixTemplateDelF(_af, _ef, node, _pf, _nf);
		vec.Del(node);
		free(node);
		nsz --;

		return ;
	}

public:

	void Clean(){
		AListAllocOListNode *node = _a, *d;
		while(node){
			d = node;
			node = node->_n;
			free(d);			
		}

		Init();
		vec.Clean();
	}

	~AListAllocOList(){
		Clean();
	}

};


// Object Allocate Fast  ( + sizeof(void*) memory per allocate element )
// Allocate: if(!free_element) Allocate node; return free_element;
// Allocate node: malloc(array of AListEl); => free_element;
// Free: element->parent -> node.used_size --; if(!node.used size) free(node);
template<class AListEl, int AListOps = AListCon | AListDes | AListClear, int blocksize = 16384>
class AListAllocOListFast{

	class AListElEx{
	public:
		AListAllocOListNode *parent;
		AListEl element;
	};

	// Nodes
	AListAllocOListNode *_a, *_e;

	// Free nodes
	AListAllocOListNode *_af, *_ef;

	// Nodes Size
	unsigned int nsz;

	// Elements Size
	unsigned int esz;

public:
	AListAllocOListFast(){
		Init();
	}

	void Init(){
		_a = 0;
		_e = 0;
		_af = 0;
		_ef = 0;
		esz = 0;
		nsz = 0;
	}

	AListEl* AllocNew(){
		if(!_af)
			AllocNode();

		AListEl *el = (AListEl*) _af->fel;
		_af->fel = _af->fel->_n;
		_af->usz ++;		

		if(!_af->fel){
			if(_af->_nf)
				_af = _af->_nf;
			else
				_af = 0;
		}

		if(AListOps & AListCon)
			new(el)AListEl;
		else if(AListOps & AListZero)
			memset(el, 0, sizeof(AListEl));

		return el;
	}

	void AllocFree(AListEl *el){
		if(!el)
			return ;

		if(AListOps & AListDes)
			el->~AListEl();

		AListAllocOListNode *node;
		node = GetNode(el);

		//if(!node)
		//	int err = 546;

		AListAllocOListFel *fel = (AListAllocOListFel*)el;
		fel->_n = node->fel;
		node->fel = fel;
		node->usz --;

		if(!node->usz)
			FreeNode(node);

		return ;
	}

private:

	void AllocNode(){
		// New
		int els = (blocksize - sizeof(AListAllocOListNode)) / sizeof(AListEl);
		AListAllocOListNode *node = (AListAllocOListNode*) malloc(sizeof(AListAllocOListNode) + els * sizeof(AListEl));
		node->Clear();
		nsz ++;

		// Add to vector
		AListAllocOListNode *vnode = 0;

		// Add to list
		//if(!vnode){
			OMatrixTemplateAdd(_a, _e, node);
		//}
		
		// Add elements to free list
		AListEl *el = (AListEl*)(node + 1), *to = el + els;
		AListAllocOListFel *fel;

		while(el < to){
			fel = (AListAllocOListFel*) el;
			fel->_n = node->fel;
			node->fel = fel;
			el ++;
		}

		OMatrixTemplateAddF(_af, _ef, node, _pf, _nf);

		return ;
	}

	AListAllocOListNode* GetNode(AListEl *el){
		AListElEx *ex = (AListElEx*)((AListAllocOListNode*)el - 1);
		return ex->parent;
	}

	void FreeNode(AListAllocOListNode *node){
		return ;

		OMatrixTemplateDel(_a, _e, node);
		OMatrixTemplateDel(_af, _ef, node);
		free(node);

		return ;
	}

public:

	void Clean(){
		AListAllocOListNode *node = _a, *d;
		while(node){
			d = node;
			node = node->_n;
			free(d);			
		}

		Init();
	}

	~AListAllocOListFast(){
		Clean();
	}

};


// ObjectTree Allocate :: WARNING !!! So Slow! Don't use.
// Allocate: if(!free_element) Allocate node; return free_element;
// Allocate node: malloc(array of AListEl); => free_element;
// Free: Binary tree -> node.used_size --; if(!node.used size) free(node);
template<class AListEl, int AListOps = AListCon | AListDes | AListClear, int blocksize = 16384>
class AListAllocOListTree{

	// Free structure ///////
	class AListAllocOListFel{
	public:
		AListAllocOListFel *_n;
	};

	// Node structure ////////
	class AListAllocOListNode{
	public:
		// Left & right (tree)
		AListAllocOListNode *left, *right;

		// Prev & Next free (linked) node
		AListAllocOListNode *pfel, *nfel;

		// Free elements
		AListAllocOListFel *fel;

		// Used size
		unsigned int usz;

		// Data
		//AListEl data[0];

		void Clear(){
			left = 0;
			right = 0;
			pfel = 0;
			nfel = 0;
			fel = 0;
			usz = 0;
		}

		AListAllocOListNode(){
			Clear();
		}

	};

	// Nodes
	AListAllocOListNode *nodes;

	// Noides Size
	unsigned int nsz;

	// Prev & Next free (linked) node
	AListAllocOListNode *pfel, *nfel;

public:
	AListAllocOListTree(){
		pfel = 0;
		nfel = 0;
		nsz = 0;
		nodes = 0;
	}

	AListEl* AllocNew(){
		if(!pfel)
			AllocNode();

		AListEl *el = (AListEl*) pfel->fel;
		pfel->usz ++;

		pfel->fel = pfel->fel->_n;

		if(!pfel->fel){
			if(pfel->pfel)
				pfel = pfel->nfel;
			else
				pfel = 0;
		}

		if(AListOps & AListCon)
			new(el)AListEl;
		else
			memset(el, 0, sizeof(AListEl));

		return el;
	}

	void AllocFree(AListEl *el){
		if(!el)
			return ;

		if(AListOps & AListDes)
			el->~AListEl();

		AListAllocOListNode *node, *pnode;
		node = GetNode(el, pnode);

		if(!node)
			int err = 67;

		AListAllocOListFel *fel = (AListAllocOListFel*)el;
		fel->_n = node->fel;
		node->fel = fel;
		node->usz --;

		if(!node->usz)
			FreeNode(node, pnode);

		return ;
	}

private:

	void AllocNode(){
		int els = (blocksize - sizeof(AListAllocOListNode)) / sizeof(AListEl);
		AListAllocOListNode *node = (AListAllocOListNode*) malloc(sizeof(AListAllocOListNode) + els * sizeof(AListEl));
		node->Clear();
		nsz ++;

		AListAllocOListNode *tnode = nodes;

		if(!nodes)
			nodes = node;
		else
			while(tnode){
				if(node < tnode){
					if(tnode->left)
						tnode = tnode->left;
					else{
						tnode->left = node;
						break;
					}
				}

			if(node > tnode){
					if(tnode->right)
						tnode = tnode->right;
					else{
						tnode->right = node;
						break;
					}
				}
		}

		// Add elements to free list
		AListEl *el = (AListEl*)(node + 1), *to = el + els;
		AListAllocOListFel *fel;

		while(el < to){
			fel = (AListAllocOListFel*) el;
			fel->_n = node->fel;
			node->fel = fel;
			el ++;
		}

		OMatrixTemplateAddF(pfel, nfel, node, pfel, nfel);
		// node->nfel = nodes->nfel;
		// nodes->nfel = node;

		return ;
	}

	void Test(){
		TestNode(nodes);
	}

	
	void TestNode(AListAllocOListNode *node){
		if(!node)
			return ;

		//if(node->left >= node || node->right <= node && node->right)
		//	int err = 64;

		TestNode(node->left);
		TestNode(node->right);
	}

	AListAllocOListNode* GetNode(AListEl *el, AListAllocOListNode *&pnode){
		AListAllocOListNode *tnode = nodes;
		pnode = nodes;

		int els = (blocksize - sizeof(AListAllocOListNode)) / sizeof(AListEl);
		int sz = sizeof(AListAllocOListNode) + els * sizeof(AListEl);

		while(tnode){
			if((char*)el < (char*)tnode + sz  && (char*)el > (char*)tnode)
				return tnode;

			if((char*)el < (char*)tnode){
				pnode = tnode;
				tnode = tnode->left;
				continue;
			}

			pnode = tnode;
			tnode = tnode->right;
		}

		return tnode;
	}

	void FreeNode(AListAllocOListNode *node, AListAllocOListNode *pnode){
		return ;

		AListAllocOListNode *rnode = 0;
		
		if(!node->left){
			rnode = node->right;
		} else
			if(!node->right)
				rnode = node->left;
			else{
				rnode = node->right;
			}

		if(pnode == nodes)
			nodes = rnode;
		else if(pnode->left == node)
			pnode->left = rnode;
		else if(pnode->right == node)
			pnode->right = rnode;

		free(node);

		return ;
	}

	void ClearNode(AListAllocOListNode *node){
		if(!node)
			return ;

		ClearNode(node->left);
		ClearNode(node->right);

		free(node);

		return ;
	}

public:

	void Clear(){
		ClearNode(nodes);

		pfel = 0;
		nfel = 0;
		nsz = 0;
		nodes = 0;

		//AListAllocOListNode::Clear();

		return ;
	}

	~AListAllocOListTree(){
		Clear();
	}

};