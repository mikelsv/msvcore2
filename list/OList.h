//NEW OBJECT MATRIX -- updated to --> Object List -- updated to --> Very new Object List

template<class OListEl, int AListOps = AListCon | AListDes | AListClear>
class OList;

template<class OListEl>
class OListP;

// My: http://pastebin.com/wxjZ3vsL

// Extended Element
template<class OListEl>
class OListElEx{
public:
	OListElEx *_p, *_n;
	OListEl el;

#ifdef OLIST_TEST
	uint64 test;
#endif
};

// Jump
template<class OListEl>
class OListJ{
private:
	// Pointers
	OList<OListEl> *olist;
	OListEl *el;

	// Locked
	int lock;

public:
	OListJ(){
		olist = 0;
		el = 0;
		lock = 0;
	}

	OListJ(OList<OListEl> *o, OListEl *e){
		olist = o;
		el = e;

		if(olist){
			o->Lock();
			lock = 1;
		} else
			lock = 0;
	}

	// Move
	OListJ(OListJ &j){
		olist = j.olist;
		el = j.el;
		lock = j.lock;

		j.lock = 0;
	}

	OListJ(OListP<OListEl> &p){
		olist = p.olist;
		el = p.el;
		
		if(olist){
			olist->Lock();
			lock = 1;
		} else
			lock = 0;
	}

	// Move
	OListJ& operator=(OListJ &j){
		olist = j.olist;
		el = j.el;
		lock = j.lock;

		j.lock = 0;
		return *this;
	}

//#if __cplusplus >= 201103L || WIN32 && !__GNUC__
	OListJ(OListJ &&j){
		olist = j.olist;
		el = j.el;
		lock = j.lock;

		j.lock = 0;
	}
//#endif

	~OListJ(){
		if(olist && lock)
			olist->UnLock();

		olist = 0;
		el = 0;
		lock = 0;
	}

	//template<class OListEl>
	friend class OListP<OListEl>;
};

// Pointer
template<class OListEl>
class OListP : public TLock{
private:
	// Pointers list in OList
	OListP *_p, *_n;

	// Pointers
	OList<OListEl> *olist;
	OListEl *el;

public:
	OListP(){
		_p = 0;
		_n = 0;
		olist = 0;
		el = 0;
	}

private:
	OListP(OListP &o){}
	OListP& operator=(OListP &o){ return this; }

public:
	OListP(const OList<OListEl> &o){
		_p = 0;
		_n = 0;
		olist = o.GetThis();
		el = 0;

		Lock();
		olist->NewLink(this);
	}

	OListP(const OListJ<OListEl> &j){
		_p = 0;
		_n = 0;
		olist = j.olist;
		el = j.el;

		Lock();
		olist->NewLink(this);
	}

	OListP& operator=(const OListJ<OListEl> &j){
		Clean();

		olist = j.olist;
		el = j.el;

		Lock();
		olist->NewLink(this);

		return *this;
	}

	OListEl* First(){
		if(!olist)
			return 0;

		UnLock();
		UGLOCK(olist);

		el = olist->First();

		Lock();

		return el;
	}

	OListEl* Next(){
		if(!olist)
			return 0;

		UnLock();
		UGLOCK(olist);

		el = olist->Next(el);

		Lock();

		return el;
	}

	OListEl* operator ->(){
		return el;
	}

	operator OListEl*(){
		return el;
	}

	void Free(){
		if(!olist)
			return ;

		OListEl *del = el;

		UnLock();
		UGLOCK(olist);

		if(del == el){
			el = olist->Next(el);
			olist->Free(del);
		}

		Lock();

		return ;
	}

	void Clean(){
		if(olist){
			UnLock();
			olist->FreeLink(this);
		}

		_p = 0;
		_n = 0;
		olist = 0;
		el = 0;
	}

	~OListP(){
		Clean();
	}

	//template<class OListEl, int AListOps>
	friend class OList<OListEl>;
	friend class OListJ<OListEl>;
};

//template<class OListEl>
//typedef int (*OLIST_SORTFUNC)(OListEl *l, OListEl *r);

template<class OListEl, int AListOps>
class OList : public AListAllocOList<OListElEx<OListEl>, AListOps>, public TLock{

	// Linked elements
	OListElEx<OListEl> *_a, *_e;

	// Linked pointers
	OListP<OListEl> *_ap, *_ep;

	// Size
	int sz;

public:
	OList(){
		_a = 0;
		_e = 0;
		_ap = 0;
		_ep = 0;
		sz = 0;
	}

	OListEl* NewE(){
		UGLOCK(this);

		OListElEx<OListEl> *el = this->AllocNew();
		OMatrixTemplateAdd(_a, _e, el);

#ifdef OLIST_TEST
		el->test = 0xfacedeadfeedcade;
#endif

		sz ++;
		return &el->el;
	}

	OListJ<OListEl> New(){
		UGLOCK(this);

		OListJ<OListEl> j(this, NewE());

		return j;
	}

	OListEl* First(){
		UGLOCK(this);

		if(!_a)
			return 0;

		return &_a->el;
	}

	OListEl* Prev(OListEl *el){
		UGLOCK(this);

		if(!_a)
			return 0;

		if(!el)
			return &_e->el;

		OListElEx<OListEl> *p = GetEx(el);

		if(p->_p)
			return &p->_p->el;

		return 0;
	}

	OListEl* Next(OListEl *el){
		UGLOCK(this);

		if(!_a)
			return 0;

		if(!el)
			return &_a->el;

		OListElEx<OListEl> *p = GetEx(el);

		if(p->_n)
			return &p->_n->el;

		return 0;
	}

	OList<OListEl>* GetThis() const{
		return (OList<OListEl>*)this;
	}

	template <typename SortFunc>
	void Sort(SortFunc func){
		UGLOCK(this);
		int sorted = 1;

		while(sorted){
			OListElEx<OListEl> *p = _a, *n;
			sorted = 0;
			
			while(p && p->_n){
				if(func(&p->el, &p->_n->el)){
					n = p->_n;
					OMatrixTemplateDel(_a, _e, p);
					OMatrixTemplateAddP(_a, _e, n, p);
					sorted = 1;
					continue;
				}
				p = p->_n;
			}
		}

		return ;
	}

	template <typename SeaFunc, class SeaClass>
	OListEl* Search(SeaFunc func, SeaClass cls){
		UGLOCK(this);

		OListElEx<OListEl> *p = _a;
		
		while(p){
			if(func(p->el, cls))
				return &p->el;

			p = p->_n;
		}

		return 0;
	}

	void Free(OListEl *el){
		UGLOCK(this);

		if(!el)
			return ;

		OListElEx<OListEl> *e = GetEx(el);

#ifdef OLIST_TEST
		if(e->test != 0xfacedeadfeedcade)
			globalerror("OMATRIX TEST FAIL!");
		e->test = 0xfacedeadfeedcada;
#endif

		OListP<OListEl> *p = _ap;
		while(p){
			if(p->el == el){
				p->Lock();
				p->el = e->_p != 0 ? &e->_p->el : 0;
				p->UnLock();
			}

			p = p->_n;
		}

		OMatrixTemplateDel(_a, _e, e);

		this->AllocFree(e);
		sz --;

		return ;
	}

private:

	OListElEx<OListEl>* GetEx(OListEl *el){
		if(!el)
			return 0;

		return (OListElEx<OListEl>*)((char*)el - 2 * sizeof(void*));
	}

	void NewLink(OListP<OListEl> *p){
		UGLOCK(this);

		if(p->_n)
			globalerror("void NewLink(OListP<OListEl> *p){ Allready pointed! }");

		OMatrixTemplateAdd(_ap, _ep, p);

		return ;
	}

	void FreeLink(OListP<OListEl> *p){
		UGLOCK(this);

		OMatrixTemplateDel(_ap, _ep, p);

		return ;
	}

public:

	unsigned int Size(){
		return sz;
	}

	void Clean(){
		UGLOCK(this);

		OListElEx<OListEl> *p = _a, *d;
		
		while(p){
			d = p;
			p = p->_n;
			this->AllocFree(d);
		}

		_a = 0;
		_e = 0;
		sz = 0;

		return ;
	}

	~OList(){
		Clean();
	}

	//template<class OListEl>
	friend class OListP<OListEl>;

};


// Single Thread
template<class OListEl, int AListOps = AListCon | AListDes | AListClear>
class OListSingle;

template<class OListEl, int AListOps>
class OListSingle : public AListAllocOList<OListElEx<OListEl>, AListOps>{

	// Linked elements
	OListElEx<OListEl> *_a, *_e;

	// Size
	int sz;

public:
	OListSingle(){
		_a = 0;
		_e = 0;
		sz = 0;
	}

	OListEl* New(){
		OListElEx<OListEl> *el = this->AllocNew();
		OMatrixTemplateAdd(_a, _e, el);

#ifdef OLIST_TEST
		el->test = 0xfacedeadfeedcade;
#endif

		sz ++;
		return &el->el;
	}

	OListEl* First(){
		if(!_a)
			return 0;

		return &_a->el;
	}

	OListEl* Prev(OListEl *el){
		if(!_a)
			return 0;

		if(!el)
			return &_e->el;

		OListElEx<OListEl> *p = GetEx(el);

		if(p->_p)
			return &p->_p->el;

		return 0;
	}

	OListEl* Next(OListEl *el){
		if(!_a)
			return 0;

		if(!el)
			return &_a->el;

		OListElEx<OListEl> *p = GetEx(el);

		if(p->_n)
			return &p->_n->el;

		return 0;
	}

	OList<OListEl>* GetThis() const{
		return (OList<OListEl>*)this;
	}

	template <typename SortFunc>
	void Sort(SortFunc func){
		int sorted = 1;

		while(sorted){
			OListElEx<OListEl> *p = _a, *n;
			sorted = 0;
			
			while(p && p->_n){
				if(func(&p->el, &p->_n->el)){
					n = p->_n;
					OMatrixTemplateDel(_a, _e, p);
					OMatrixTemplateAddP(_a, _e, n, p);
					sorted = 1;
					continue;
				}
				p = p->_n;
			}
		}

		return ;
	}

	template <typename SeaFunc, class SeaClass>
	OListEl* Search(SeaFunc func, SeaClass cls){
		OListElEx<OListEl> *p = _a;
		
		while(p){
			if(func(p->el, cls))
				return &p->el;

			p = p->_n;
		}

		return 0;
	}

	void Free(OListEl *el){
		if(!el)
			return ;

		OListElEx<OListEl> *e = GetEx(el);

#ifdef OLIST_TEST
		if(e->test != 0xfacedeadfeedcade)
			globalerror("OMATRIX TEST FAIL!");
		e->test = 0xfacedeadfeedcada;
#endif

		OMatrixTemplateDel(_a, _e, e);

		this->AllocFree(e);
		sz --;

		return ;
	}

private:

	OListElEx<OListEl>* GetEx(OListEl *el){
		if(!el)
			return 0;

		return (OListElEx<OListEl>*)((char*)el - 2 * sizeof(void*));
	}

public:

	unsigned int Size(){
		return sz;
	}

	void Clean(){

		OListElEx<OListEl> *p = _a, *d;
		
		while(p){
			d = p;
			p = p->_n;
			this->AllocFree(d);
		}

		_a = 0;
		_e = 0;
		sz = 0;

		return ;
	}

	~OListSingle(){
		Clean();
	}

	//template<class OListEl>
	friend class OListP<OListEl>;

};