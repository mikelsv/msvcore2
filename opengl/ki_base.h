#define KI_PHY_POW2(v) ((v) * (v))

class KiVec2{
public:
	float x, y;

	KiVec2(){
		x = 0;
		y = 0;
	}

	KiVec2(float v){
		x = v;
		y = v;
	}

	KiVec2(float x, float y){
		this->x = x;
		this->y = y;
	}

	KiVec2(VString line){
		x = PartLine(line, line, ",").tod();
		y = PartLine(line, line, ",").tod();
	}

	KiVec2 operator+(KiVec2 v){
		return KiVec2(x + v.x, y + v.y);
	}

	KiVec2 operator-(KiVec2 v){
		return KiVec2(x - v.x, y - v.y);
	}

	KiVec2 operator*(float v){
		return KiVec2(x * v, y * v);
	}

	KiVec2 operator/(float v){
		return KiVec2(x / v, y / v);
	}

	bool Is(){
		return x != 0 || y != 0;
	}

	bool IsNull(){
		return x == 0 && y == 0;
	}

};

class KiVec3{
public:
	float x, y, z;

	KiVec3(){
		x = 0;
		y = 0;
		z = 0;
	}

	KiVec3(float v){
		x = v;
		y = v;
		z = v;
	}

	KiVec3(const KiVec2 v){
		x = v.x;
		y = v.y;
		z = 0;
	}

	KiVec3(float _x, float _y, float _z){
		x = _x;
		y = _y;
		z = _z;
	}

	KiVec3 Normalize(){
		float m = max(abs(x), max(abs(y), abs(z)));
		return KiVec3(x / m, y / m, z / m);
	}

	float Length(){
		return sqrt(KI_PHY_POW2(x) + KI_PHY_POW2(y) + KI_PHY_POW2(z));
	}

	KiVec3 operator+(KiVec3 v){
		return KiVec3(x + v.x, y + v.y, z + v.z);
	}

	KiVec3 operator-(KiVec3 v){
		return KiVec3(x - v.x, y - v.y, z - v.z);
	}

	KiVec3 operator*(float v){
		return KiVec3(x * v, y * v, z * v);
	}

	KiVec3 operator/(float v){
		return KiVec3(x / v, y / v, z / v);
	}

	KiVec3& operator+=(KiVec3 v){
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	KiVec3& operator-=(KiVec3 v){
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	KiVec3& operator*=(float v){
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}

	KiVec3& operator/=(float v){
		x /= v;
		y /= v;
		z /= v;

		return *this;
	}

	bool Is(){
		return x + y + z != 0;
	}

};

class KiInt2{
public:
	int x, y;

	KiInt2(){
		x = y = 0;
	}

	KiInt2(int _x, int _y){
		x = _x;
		y = _y;
	}

	KiInt2(VString line){
		x = PartLine(line, line, ",").toi();
		y = PartLine(line, line, ",").toi();
	}

	KiInt2 operator+(KiInt2 v){
		return KiInt2(x + v.x, y + v.y);
	}

	KiInt2 operator-(KiInt2 v){
		return KiInt2(x - v.x, y - v.y);
	}

	KiInt2 operator*(double v){
		return KiInt2(x * v, y * v);
	}

	KiInt2 operator/(double v){
		return KiInt2(x / v, y / v);
	}

	KiInt2& operator ()(int _x, int _y){
		x = _x;
		y = _y;
		return *this;
	}

	KiInt2& operator +=(const KiInt2 v){
		x += v.x;
		y += v.y;
		return *this;
	}

	KiInt2& operator -=(const KiInt2 v){
		x -= v.x;
		y -= v.y;
		return *this;
	}

	int Length(const KiInt2 poi = KiInt2()){
		return (int)sqrt(float(KI_PHY_POW2(x - poi.x) + KI_PHY_POW2(y - poi.y)));
	}

};

class KiInt3{
public:
	int x, y, z;

	KiInt3(){}

	KiInt3(int _x, int _y, int _z){
		x = _x;
		y = _y;
		z = _z;
	}

	int Length(const KiInt3 poi){
		return (int)sqrt(float(KI_PHY_POW2(x - poi.x) + KI_PHY_POW2(y - poi.y) + KI_PHY_POW2(z - poi.z)));
	}

};