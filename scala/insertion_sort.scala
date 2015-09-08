#!/bin/sh
exec scala "$0" "$@"
!#

import scala.util.Random

val arr = 1.to(10)

Random.setSeed(1)


val arr_to_sort = Random.shuffle(arr)
println(arr_to_sort)

def insertionSort(v: IndexedSeq[Int]) : IndexedSeq[Int] = {
	for(j <- 1 until v.length) {
		val key = v(j)
		val i = j - 1
		while(i>0 && v(i)>key) {
			v(i+1) = v(i)			
		}
 	}
 	return v
}

insertionSort(arr_to_sort)
