2. Sửa File Parser.c tại hàm compileVarDecl , sau khi đọc 1 ident thì nếu kí tự tiếp theo là dấu phẩy thì sẽ đọc dấu phẩy và ident tiếp tục lặp cho đến khi gặp dấu “=” và “:”, và cho logic cũ vào của “:”, còn logic của “=” sẽ đọc dấu “=” và gọi đến hàm compileConstant

3. 
