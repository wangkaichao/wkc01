/* 
* queue_r.h
* wangkaichao2@163.com2018-09-23
*/
#ifndef _QUEUE_R
#define _QUEUE_R

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class queue_r {
	private:
		mutable std::mutex mut;
		std::queue<T> data_queue;
		std::condition_variable data_cond;

	public:
		queue_r() {}

		queue_r(const queue_r& other) {
			std::lock_guard<std::mutex> lk(other.mut);
			data_queue = other.data_queue;
		}

		queue_r& operator= (const queue_r&) = delete;

		void push(T new_value) {
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(new_value);
			data_cond.notify_one();
		}

		bool try_pop(T& value) {
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
				return false;

			value = data_queue.front();
			data_queue.pop();
			return true;
		}

		std::shared_ptr<T> try_pop() {
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
				return std::shared_ptr<T>();

			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}

		void wait_pop (T& value) {
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this]{return !data_queue.empty();});
			value = data_queue.front();
			data_queue.pop();
		}

		std::shared_ptr<T> wait_pop() {
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this]{return !data_queue.empty();});
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}

		bool empty() const {
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}
};

#endif
