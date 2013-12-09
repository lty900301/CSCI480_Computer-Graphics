Assignment #3: Ray tracing

FULL NAME: Tianyi Luo

Note: I am using the late day policy. Therefore, I submitted the homework two days after the due date.


MANDATORY FEATURES
------------------

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  yes, functioning correct

2) Ray tracing sphere                     yes, functioning correct

3) Triangle Phong Shading                 yes, functioning correct

4) Sphere Phong Shading                   yes, functioning correct

5) Shadows rays                           yes, functioning correct

6) Still images                           yes
   
7) Extra Credit (up to 20 points)
   	A. Recursive reflection (10 points)
   		I implemented the recursive reflection in the program. The recursion level is 3 currently, and you can modify this number in program by searchcing "#define RECURSION_LEVEL 3" (LINE 23).

   		I disabled Recursive reflection by default, since it is computationally intensive. To open this feature, please attach an argument "-r" when you run the program. For example, "./assign3 screenfile.txt -r" to render in regular mode. "./assign3 screenfile.txt 000.jpg -r" to render in save picture mode.
   	B. Good antialiasing (10 points)
   		I implemented the antialiasing by using super sampling method.

		Because it is very computationally intensive, I disabled this feature by default. To open this feature, please modify the SAMPLING_PIXEL integer by searching "#define SAMPLING_PIXEL 1" (LINE 22). I recommend to change the value to 3. Otherwise if larger, the computation will take too much time.

		As we can see from 002.jpg, 005.jpg, 007.jpg, and 009.jpg compared to their W/O antialiasing versions. This implementation can be counted as a good antialiasing.

Still Image Description:
------------------------
File Name 		Scene File		Recursive Reflection?		Antialiasing?
---------		----------		---------------------		-------------
000.jpg			test1.scene 			no 						no
001.jpg			test2.scene 			no 						no
002.jpg			test2.scene 			yes						yes
003.jpg			spheres.scene 			no 						no
004.jpg			spheres.scene 			yes 					no
005.jpg			spheres.scene 			yes 					yes
006.jpg 		table.scene 			no 						no
007.jpg 		table.scene 			yes 					yes
008.jpg			SIGGRAPH.scene 			no 						no
009.jpg			SIGGRAPH.scene 			yes 					yes