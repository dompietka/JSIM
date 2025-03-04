int n, m;
int dx[]={1, 0 -1, }, dy[]={0, 1, 0, -1};
vector <vector<bool>> odw;

bool ok(int x, int y, int level){
	return x >= 0 && x <= level && x < n && y >=0 && y < m;
}

//level to najnizszy na ktorym moge byc 

bool check(int level, int i, int j, vector<vector<bool>> &mapa, vector<vector<bool>> &odw){
	odw[i][j]=1;
	if(i==0) return 1;
	
	
	for (int it=0;it<4;it++){
		if (ok(i+dx[it], j+dy[it], level) && !odw[i+dx[it]][j+dy[it]] && mapa[i+dx[it]][j+dy[it]]==1 && check(level, i+dx[it], j+dy[it], mapa, odw)) return 1;
	}
	return 0;
} 

void clear_odw(){
	for (int i=0;i<n;i++){
		for (int j=0;j<m;j++)
			odw[i][j]=0;
	}
}

int kopalnia(vector<vector<bool>> &mapa, int i, int j){
	n = mapa.size()-1;
	m = mapa[0].size()-1;
	
	for (int i=0;i<n;i++){
		odw.push_back({});
		for (int j=0;j<m;j++)
			odw[i].push_back(0);
	}
	
	int l = 0;
	int h = n;
	int mid;

	
	while (l<h){
		mid = (l+h)/2;
		if (check(n-mid, i, j, mapa, odw)) l=mid+1;
		else h=mid;
	}
	
	return l;
}

int main(){
	cin >> n >> m;
	int i,j;
	cin >> i >> j;
	
	for (int i=0;i<n;i++){
		for (int j=0;j<m;j++){
		cin >> mapa[i][j];
		}
	}
	
	cout << kopalnia(mapa, i, j);
