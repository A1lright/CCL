void func(int a[][2]) {    // 形参为二维数组，第一维空缺
    a[1][0] = 100;         // 修改实参数组元素
}

int main() {
    int arr[2][2] = {{1, 2}, {3, 4}};
    func(arr);             // 传递二维数组首地址
    printf("%d\n", arr[1][0]); // 输出修改后的值
    return 0;
}