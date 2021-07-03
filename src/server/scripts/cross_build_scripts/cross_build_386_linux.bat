set GOARCH=386
set GOOS=linux
set CGO_ENABLED=1
go build -o server_386_linux ../src/ 