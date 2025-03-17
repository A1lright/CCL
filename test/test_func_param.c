// 测试不同形参类型
int func1(int a) { return a; }
void func2(int m[][3]) {}
int main() {
    int arr[2][3];
    func2(arr);
    return func1(5);
}