//Through lexical and syntatic analysis

void modify(int a[]) {
    a[0] = 99;
}

int main() {
    int arr[2][2] = {{1,2}, {3,4}};
    modify(arr[1]);  // 传递二维数组的一维
    return arr[1][0]; // 应返回99
}