// ImGui CharId
template<int size>
class ImGuiCharId {
	char str[size + 4];

public:
	ImGuiCharId(const char(&input)[size]) {
		memcpy(str, input, size);
		str[size] = '\0';
	}

	void SetId(int id) {
		str[size - 1] = 48 + (id / 1000) % 10;
		str[size + 0] = 48 + (id / 100) % 10;
		str[size + 1] = 48 + (id / 10) % 10;
		str[size + 2] = 48 + (id / 1) % 10;
		str[size + 3] = 0;
	}

	void SetId2(int id) {
		str[size - 1] = 48 + (id / 10) % 10;
		str[size + 0] = 48 + (id / 1) % 10;
		str[size + 1] = 0;
	}

	operator char* () {
		return str;
	}
};

//template <int size>
//ImGuiCharId(const char(&data)[size]) -> ImGuiCharId<size>;

// ImGui CharId Ext
template<int maxsize>
class ImGuiCharIdExt {
	char data[maxsize + 1]; // +1 for safe
	int size;


public:
	// Init
	ImGuiCharIdExt() {		
		Clean();
	}

	ImGuiCharIdExt(const char* str) {
		SetStr(str);
	}

	// Get 
	int GetSize() {
		return size;
	}

	int GetMaxSize() {
		return maxsize;
	}

	int GetFreeSize() {
		return maxsize - size;
	}

	// Int
	void SetInt(int val) {
		Clean();
		AddInt(val);
	}

	void AddInt(int val) {
		int len = GetIntLen(abs(val));
		int pow = Pow10(len - 1);
		int pos = size;
		int count = 0;
		
		// -
		if (val < 0 && pos < maxsize) {
			val *= -1;
			data[pos] = '-';
			pos++;
		}

		// Zero
		if (val == 0 && pos < maxsize) {
			data[pos] = '0';
			pos++;
		}

		// Int
		while (count < len && pos < maxsize && pow != 0) {
			data[pos] = 48 + (val / pow) % 10;
			pow /= 10;
			count++;
			pos++;
		}

		// Close
		size = pos;
		data[size] = 0;
	}

	int GetIntLen(int val) {
		int count = 0;

		while (val > 0) {
			val /= 10;
			count++;
		}

		return count;
	}

	int Pow10(int val) {
		int res = 1;

		while (val > 0) {
			res *= 10;
			val--;			
		}

		return res;
	}

	// Float
	void SetFloat(float val) {
		Clean();
		AddFloat2(val);
	}

	void AddFloat2(float val) {
		AddInt(val);

		if (val < 0)
			val *= -1;

		if (size < maxsize) {
			data[size] = '.';
			size++;
		}

		AddInt(int(val * 10) % 10);
		AddInt(int(val * 100) % 10);
	}

	// Str
	void SetStr(VString str) {
		Clean();
		AddStr(str);
	}

	void AddStr(VString str) {
		if (!str)		
			return;
	
		int len = str.size();
		if (len > maxsize - size)
			len = maxsize - size;

		memcpy(data + size, str, len);
		size += len;
		data[size] = '\0';
	}

	// Move
	void MoveLeft(int count) {
		if (size < count)
			count = size;

		memcpy(data, data + count, size - count);
		size -= count;
		data[size] = '\0';
	}

	// Check
	void CheckStrSize() {
		data[maxsize] = 0;
		size = strlen(data);
	}

	// Data
	VString GetStr() {
		return VString(data, size);
	}

	operator unsigned int() {
		return size;
	}

	operator char* () {
		return data;
	}

	// Clean
	void Clean() {
		size = 0;
		data[size] = 0;
	}
};