// NEW IDEAL MATRIX -- updated to -> Ideal List

// Use Constructor & Destructor
#define ILIST_OPT_CON	1
#define ILIST_OPT_DES	2

template<class Item, int opt = ILIST_OPT_CON|ILIST_OPT_DES>
class IList{
private:
	// Data
	Item *data;

	// Size
	unsigned int sz;

	// All Size
	unsigned int asz;
		
	// Base Size
	unsigned int bsz;

public:

	IList(int bs = 1024){
		data = 0;
		sz = 0;
		asz = 0;
		bsz = bs;
	}

	bool Add(unsigned int s = 0){
		unsigned int ns = maxel((asz + bsz), (asz + asz / 10));

		if(ns < s)
			ns = s;

		data = (Item*)AddVData(data, asz * sizeof(Item), ns * sizeof(Item));

		if(opt & ILIST_OPT_CON)
			for(unsigned int i = asz; i < asz + ns; i ++)
				new(&data[i])Item;

		asz += ns;
		return 1;
	}

	void* AddVData(void *v, int sz, int nsz){
		void *nw = malloc(sz + nsz);
		if(sz)
			memcpy((char*)nw, (char*)v, sz);

		memset((char*)nw + sz, 0, nsz);

		free(v);
		
		return nw;
	}

	bool A(){
		if(sz == asz)
			return Add();
		return 0;
	}

	bool A(int s){
		if(sz + s > asz)
			return Add(sz);
		return 0;
	}

	bool Del(unsigned int pos){
		if(pos >= sz)
			return 0;

		if(opt & ILIST_OPT_DES)
			(data + pos)->~Item();

		memcpy(data + pos, data + pos + 1, (sz - pos - 1) * sizeof(Item));

		sz --;
		return 1;
	}

	Item& operator [](unsigned int i){
		return data[i];
	}
	
	Item& n(){
		A();
		return data[sz];
	}

	void Added(){
		sz++;
		return ;
	}

	Item* First(){
		return data;
	}

	Item* Pos(int pos){
		if(!data)
			return 0;

		return data[pos];
	}

	Item* End(){
		if(!data)
			return 0;

		return data + sz;
	}

	unsigned int Size(){
		return sz;
	}

	unsigned int ASize(){
		return asz;
	}

	void Clear(){ 
		if(opt & ILIST_OPT_DES)
			for(unsigned int i = 0; i < sz; i ++)
				data[i].~Item();
		
		free(data);

		data = 0;
		asz = 0;
		sz = 0;

		return ;
	}

	~IList(){
		Clear();
	}

};