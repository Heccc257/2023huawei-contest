#include <bits/stdc++.h>

using namespace std;

typedef long long cost_t;



const cost_t NEW_EDGE_COST = 1000000;
const cost_t AMPLIFIER_COST = 100;
const cost_t BIGCOST = 1ll<<60;
const int MAXN = 5005;
const int MAXM = 5005;
const int MAXT = 10005;
const int MAXP = 80;
const int MAX_NEW_EDGE_NUM = 20000;

int N, M, T, P, D;
random_device global_rd;

struct edge {
    int s, t;
    int d;
	int id;
	edge() = default;
    edge(int _s, int _t, int _d, int _id): s(_s), t(_t), d(_d), id(_id) { }
};
struct business {
	int s, t;
	int id;
	business(int _s, int _t, int _id): s(_s), t(_t), id(_id) { }
};

vector<edge>edges;

vector<business> businesses;

/**
 * prefix
 * edge_id
 */
struct trace_entry {
	int prefix, edge_id;
	trace_entry() = default;
	trace_entry(int _prefix, int e_id): prefix(_prefix), edge_id(e_id) { }
};

struct Answer {
	cost_t tot_cost;
	vector<edge>new_edges;
	vector<int>amplifiers;
	// 对每个business,记录下trace就行,然后再还原出放大器的放置
	vector<vector<trace_entry> > traces;
	vector<int> channels;

	void add_trace(int bid, int channel_id, vector<trace_entry>& trace) {
		// if (trace.size() == 0) exit(-1);

		int s = businesses[bid].s;
		int t = businesses[bid].t;
		traces[bid].clear();
		while (t != s) {
			traces[bid].push_back(trace[t]);
			t = trace[t].prefix;
		}
		channels[bid] = channel_id;
	}

	void add_new_edge(edge e) {
		new_edges.emplace_back(e);
	}
	Answer() {
		tot_cost = BIGCOST; 
		traces.resize(T);
		channels.resize(T);
	}

	~Answer() {
		new_edges.clear();
		new_edges.shrink_to_fit();
		traces.clear();
		traces.shrink_to_fit();
	}

	cost_t Cost() {
		cost_t ans = 0;
		ans += new_edges.size() * NEW_EDGE_COST;
		for (auto trace: traces) {
			ans += (trace.size() - 1) * AMPLIFIER_COST;
		}
		return ans;
	}

	void place_amplifier(vector<trace_entry>& trace, vector<int>&ans) {
		ans.clear();
		int len = 0;
		for (auto e = trace.rbegin(); e != trace.rend(); e++) {
			// cout << "len = " << len << " d = " << edges[e->edge_id].d << " pre = " << e->prefix << '\n';
			if (len + edges[e->edge_id].d > D) {
				len = edges[e->edge_id].d;
				ans.push_back(e->prefix);
				// printf("%d ", e->prefix);
			} else len += edges[e->edge_id].d;
		}
	}
	void Print() {
		printf("%d\n", (int)new_edges.size());
		for (auto &e: new_edges) {
			printf("%d %d\n", e.s, e.t);
		}

		for (int i=0; i<T; i++) {
			auto &trace = traces[i];
			place_amplifier(trace, amplifiers);
			printf("%d %d %d ", channels[i], (int)trace.size(), (int)amplifiers.size());
			// cerr << "i= " << i<< '\n';
			// cout << channels[i] << ' ' << trace.size() << ' ' << trace.size() - 1 << ' ';
			for (auto e = trace.rbegin(); e != trace.rend(); e++)
				printf("%d ", e->edge_id);
			for (auto &a: amplifiers)
				printf("%d ", a);
			puts("");
		}
	}
};

struct graph {
	struct dir_edge;
	int channel_id;

	// 每个图单独一个trace，跑完dij之后trace会存每个点
	// 的最短路来源，根据这个来将边删除
	vector<trace_entry>trace;

	// 表示某条边被占了
	// 只能加边 不能删边
	vector<bool>occupied;

	// 无相边拆成两条有向边
	// 因为所有图的边都是一样的，所以用static存
	// 要加边也直接加
	
	// TODO 加边似乎可以不用真的插入一条新的边，而是
	// 给原有的边增加一个类似于生命值属性
	static vector<vector<dir_edge> >des;
	int origin_edges_num;
	struct dir_edge {
		int to, d, id;
		dir_edge(int _to, int _d, int _id): to(_to), d(_d), id(_id) { }
	};
	void add_edge(int s,int t, int d, int id) {
		des[s].emplace_back(dir_edge(t, d, id));
		des[t].emplace_back(dir_edge(s, d, id));
	}
	void init(int n, int m, int id, vector<edge>&edges) {
		channel_id = id;
		origin_edges_num = m;
		
		trace.resize(n);

		if (channel_id == 0) {
			des.clear();
			des.resize(n);
			for (auto &e: edges)
				add_edge(e.s, e.t, e.d, e.id);
		}

		occupied.clear();
		occupied.resize(m);

	}
	
	bool vis[MAXN];
	cost_t f[MAXN];
	struct node {
		int v;
		cost_t f;
		node(int _v, cost_t _f):v(_v), f(_f) { }
		bool operator < (const node &rhs)const {
			return f > rhs.f;
		}
	};

	cost_t dijkstra(int s, int t, cost_t threshold) {
		memset(vis, 0, sizeof(vis));
		memset(f, 0x3f, sizeof(f));
		f[s] = 0;
		priority_queue<node>q;
		q.push(node(s, f[s]));

		while (!q.empty()) {
			int v = q.top().v;
			q.pop();
			if (vis[v]) continue ;
			vis[v] = 1;
			if (f[v] > threshold) continue;
			for (auto &e: des[v]) {
				int to = e.to;
				if (vis[to]) continue ;
				cost_t val = f[v] + e.d + (occupied[e.id] == 1) * NEW_EDGE_COST;
				if (val < f[to]) {
					trace[to] = trace_entry(v, e.id);
					f[to] = val;
					q.push(node(to, f[to]));
				}
			}
		}
		return f[t];
	}


	int new_edges[MAXN];
	void bfs_least_new_edges(int s, int t) {
		static queue<int>q[MAXN];
		memset(new_edges, 0x3f, sizeof(new_edges));
		new_edges[s] = 0;
		q[0].push(s);
		for(int idx=0; !q[idx].empty(); idx++) {
			while (!q[idx].empty()) {
				int v = q[idx].front();
				q[idx].pop();
				if (new_edges[v] != idx || new_edges[v] > new_edges[t]) continue ;
				for (auto &e: des[v]) {
					int to = e.to;
					if (f[v] + (occupied[e.id] == 1) < f[to]) {
						f[to] = f[v] + (occupied[e.id] == 1);
						q[f[to]].push(to);
					}
				}
			}
		}
	}
};

vector<vector<graph::dir_edge> > graph::des;

struct graph_manageer {
	graph gr[MAXP];
	void init() {
		for (int i=0; i<P; i++) {
			gr[i].init(N, M, i, edges);
		}
	}
	void reset() {
		edges.resize(M);
		for (int i=0; i<P; i++)
			gr[i].init(N, M, i, edges);
	}

	int add_edge_for_all_graph(int s, int t, int d, Answer &ans) {
		int id = edges.size();
		
		gr[0].add_edge(s, t, d, id);
		for (int i=0; i<P; i++) {
			gr[i].occupied.push_back(0);
		}
		edges.push_back(edge(s, t, d, id));
		ans.add_new_edge(edge(s, t, d, id));
		return id;
	}

	void Modify(vector<bool>& occupied, vector<trace_entry>& trace, int s, int t, Answer& final_ans) {
		int temt = t;
		while (temt != s) {
			int eid = trace[temt].edge_id;
			if (occupied[eid]) {
				trace[temt].edge_id = add_edge_for_all_graph(edges[eid].s, edges[eid].t, edges[eid].d, final_ans);
			}
			temt = trace[temt].prefix;
		}
		temt = t;
		while (temt != s) {
			occupied[trace[temt].edge_id] = 1;
			temt = trace[temt].prefix;
		}
	}

	void two_steps(Answer& final_ans) {
		vector<int>rest_business;
		unsigned long long lst = global_rd();
		cost_t ans = BIGCOST;
		int best_channel;
		for (auto &bs: businesses) {
			ans = BIGCOST;

			unsigned int c = lst % P;
			for (int j=0; j<min(P, 20); j++) {
				cost_t tem = gr[c].dijkstra(bs.s, bs.t, ans);
				if (tem < ans) {
					ans = tem;
					best_channel = c;
				}
				c = (c+1 == (unsigned int)P) ? 0 : c+1;
			}
			if (ans < NEW_EDGE_COST) {
				lst = lst + ans;
				Modify(gr[best_channel].occupied, gr[best_channel].trace, bs.s, bs.t, final_ans);
				final_ans.add_trace(bs.id, best_channel, gr[best_channel].trace);
			} else {
				rest_business.push_back(bs.id);
			}
		}
		for (auto &res: rest_business) {
			auto &bs = businesses[res];
			
		}
	}
	void baoli(Answer& final_ans);
}gm;

void Input() {
	scanf("%d%d%d%d%d", &N, &M, &T, &P, &D);
	int s, t, d;
	for (int i=0; i<M; i++) {
		scanf("%d%d%d", &s, &t, &d);
		// if (s == t) exit(-1);
		edges.emplace_back(edge(s, t, d, i));
	}
	for (int i=0; i<T; i++) {
		scanf("%d%d", &s, &t);
		businesses.emplace_back(business(s, t, i));
	}
}

int main() {
    Input();

	Answer final_ans;
	gm.init();
	gm.baoli(final_ans);
	// gm.two_steps(final_ans);
	final_ans.Print();
    return 0;
}

void graph_manageer::baoli(Answer& final_ans) {
	unsigned long long lst = global_rd();
	// cerr << "lst = " << lst << '\n';
	cost_t ans = BIGCOST;
	int best_channel;

	// 当时可有调到40,
	int max_epoches = 5;
	for (auto &bs: businesses) {
		ans = BIGCOST;
		unsigned int c = lst % P;
		for (int j=0; j<min(P, max_epoches); j++) {
			cost_t tem = gr[c].dijkstra(bs.s, bs.t, ans);
			if (tem < ans) {
				ans = tem;
				best_channel = c;
			}
			c = (c+1 == (unsigned int)P) ? 0 : c+1;
		}

		lst = lst + ans;
		Modify(gr[best_channel].occupied, gr[best_channel].trace, bs.s, bs.t, final_ans);
		final_ans.add_trace(bs.id, best_channel, gr[best_channel].trace);
	}
}