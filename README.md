# Hash Cracker
Hash cracker I made in my CS4000 course that uses multithreading to generate every possible combination, convert it to a hash, check to see if it matches the target hash &amp; return whether the password is found or not.

## Proper installation

This project requires the use of OpenMPI to work.

To install OpenMPI, run the following command in your Linux Environment

```bash
sudo apt install libopenmpi-dev
```

### How to use

Once you have OpenMPI installed, you can use the makefile to get an idea of how this project is supposed to run

The command below will create the executable for you to use if you would like
```bash
make all
```

After running the command above, you are able to run the executable and you will be prompted to enter the salt, target hash, alphabet, and password length, for example you can enter something like:
```bash
foobar 	 9pHdGraWcEy3y.NvdzCOSfu0XalZhBWUgJ/iKxpdipC  01  8
```

To run this project with multiple processes, you can type the command:
```bash
mpiexec -n 4 ./hash_cracker
```
Where 4 is the number of processes.

You may also run this command below get an idea of how it works with the test cases used for the project
```bash
make test
```


