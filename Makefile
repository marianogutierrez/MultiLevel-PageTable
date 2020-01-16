#Cameron Ozatalar
#Mariano Gutierrez
# going to need trace reader 
# : indicates dependencies 
# the header file and  just need to link that thing 
pagetable: pagetable.c pagetableDS.o byu_tracereader.o
	gcc pagetable.c pagetableDS.o byu_tracereader.o -g -o pagetable -lm -std=c99 

# need to link up the byu methodsls
byu_tracereader.o: byu_tracereader.c byutr.h
	gcc -c byu_tracereader.c -g -std=c99

#Create pagetableDS object file
pagetableDS.o: pagetableDS.c pagetableDS.h
	gcc -c pagetableDS.c -g -lm -std=c99

#Clean up object and executable files; Note that core is for the core dump file if one is produced
clean:
	rm -f core pagetableDS.o LinkedList.o byueg.o pagetable