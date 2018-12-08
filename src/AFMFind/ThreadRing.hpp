/**
 * \class ThreadRing 
 *
 * This class is a concurrent task queue based on a thread pool.

 */

#ifndef NREP_THREAD_RING_HPP
#define NREP_THREAD_RING_HPP

#include <vector>
#include <memory>

#include "misc/ThreadPool.h"


namespace nrep {

	using std::vector;

	template<class T>
	class WTask {
		
		template<typename U>
		friend class ThreadRing;
		
		int id;
		std::shared_ptr<T> m_payload;

		void setId(int id_) {
			id = id_;
		}

		int operator()() {
			(*m_payload)();
			return id;
		}

	public:
		
		std::shared_ptr<T> getPayload() {
			return m_payload;
		}

		void setPayload(std::shared_ptr<T> payload) {
			m_payload = payload;
		}			
	};

	template<class T>
	class ThreadRing {

		vector<std::shared_ptr<WTask<T> > > tasks;
		std::queue<int> taskFlags;

		std::queue< std::future<int> > results;

		ThreadPool tPool;

	public:

		ThreadRing(int numThreads) {

			tPool.init(numThreads);

		}

		~ThreadRing() {
			//cout << endl << "Destroying ThreadRing ..." << endl;
		}

		vector<std::shared_ptr<WTask<T> > >& getTasks() {
			return tasks;
		}

		void submitNewTask(std::shared_ptr<T> w) {

			// Make new payload
			auto t = std::make_shared<WTask<T>>();
			t->setPayload(w);
			t->setId(static_cast<int>(tasks.size()));
			tasks.push_back(t);
			// Submit
			results.push(tPool.enqueue(&WTask<T>::operator(), t));
		}

		void submitTask(std::shared_ptr<T> w) {
			int idx = taskFlags.front();
			auto t = tasks[idx];
			t->setPayload(w);
			results.push(tPool.enqueue(&WTask<T>::operator(), t));
			taskFlags.pop();
		}

		std::shared_ptr<T> nextTask() {
			int t = results.front().get();
			results.pop();
			taskFlags.push(t);
			return tasks[t]->m_payload;
		}

		int size() {
			return static_cast<int>(results.size());
		}


	};

}


#endif


