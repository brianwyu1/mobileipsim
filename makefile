all: dataSource mobileNode homeAgent foreignAgent
dataSource:dataSource.c 
	gcc -Wall -Wextra dataSource.c -o dataSource
mobileNode:mobileNode.c
	gcc -Wall -Wextra mobileNode.c -o mobileNode	
foreignAgent:foreignAgent.c
	gcc -Wall -Wextra foreignAgent.c -o foreignAgent
homeAgent:homeAgent.c
	gcc -Wall -Wextra homeAgent.c -o homeAgent		
