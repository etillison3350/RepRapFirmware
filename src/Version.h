/*
 * Version.h
 *
 *  Created on: 25 Dec 2016
 *      Author: David
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_


#ifndef VERSION
#ifdef RTOS
# define MAIN_VERSION	"2.05+E3350"
#else
# define MAIN_VERSION	"1.26"
#endif

# define VERSION MAIN_VERSION
#endif

#ifndef DATE
# define DATE "2021-02-18-5a"
#endif

#define AUTHORS "ricky3350, reprappro, dc42, chrishamm, t3p3, dnewman, printm3d"

#endif /* SRC_VERSION_H_ */
