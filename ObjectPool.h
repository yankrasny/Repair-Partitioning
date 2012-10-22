#include<vector>

template<class T>
class ObjectPool
{
private:
	std::vector<T*> pool;
	size_t numInUse;
	size_t capacity;
public:
	ObjectPool(size_t capacity)
	{
		numInUse = 0;
		pool = std::vector<T*>(capacity);
		for (size_t i = 0; i < capacity; i++)
		{
			pool[i] = new T(); //T must have a default constructor
		}
	}

	T* getNew()
	{
		if (numInUse < capacity - 1)
			return pool[numInUse++];

		return NULL;
	}

	void destroy(T* victim)
	{
		if (!victim)
			return;

		std::swap(victim, pool[numInUse - 1]);
		pool[numInUse - 1]->~T();
		//pool[numInUse - 1]->T();
	}
};