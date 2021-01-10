all:
	g++ -o G_VRP-Greedy+TS G_VRP-Greedy+TS.cpp
	./G_VRP-Greedy+TS instances/Large\ VA\ Input_250c_21s.txt
	#./G_VRP-Greedy+TS instances/20c3sU1.txt
