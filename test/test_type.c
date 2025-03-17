void func(int a[]) {}
int main() {
    int arr[2][2];
    func(arr); // 应报错：二维数组传递给一维形参
    return 0;
}