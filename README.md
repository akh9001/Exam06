# Exam06
This is exam is about a program that will listen for client to connect on a certain port on 127.0.0.1 and will let clients to speak with each other.

# Running the program :

```bash
clang -Werror -Wextra -Wall mini_serv.c; ./a.out 1221 
```

# For testing ( connect with a client ) :

```bash
$ nc 127.0.0.1 1221

$ nc 127.0.0.1 1221 < file_name
```
- With the first cmd (nc 127.0.0.1 1221) you will be able to connect to the serve and send messages,
  use Ctrl+C to disconnect.
- The second command, the client will connect and send the content of file_name then disconnect !
- Try to connect and disconnect several times & check if entering/leaving message the client is well displayed(right client id)!

Good Luck,
made with :heart:,
Asmaa KHALIDY.
