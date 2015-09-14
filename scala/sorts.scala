#!/usr/bin/env scalas
 
/***         
scalaVersion := "2.11.7"
 
libraryDependencies +=  "com.typesafe.scala-logging" %% "scala-logging" % "3.1.0"
libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.1.2"
*/

import com.typesafe.scalalogging.Logger
import scala.util.Random
import org.slf4j.LoggerFactory
import scala.collection.mutable.ArrayBuffer

val log = Logger(LoggerFactory.getLogger("MainLog"))

var arr = ArrayBuffer[Int]()
1.to(20).foreach(arr += _)

Random.setSeed(1)
arr = Random.shuffle(arr)

def insertionSort(v: ArrayBuffer[Int]) : ArrayBuffer[Int] = {
	log.debug(v.toString())
	for(j <- 1 until v.length) {
		log.debug("%d iteration =================================" format j)
		val key = v(j)
		var i = j - 1 // Insert v(j) into the sorted seq 0 .. j-1
		while(i>=0 && v(i)>key) {
			v(i+1) = v(i)
			log.debug("i: %d" format i)
			log.debug(v.toString())
			
			i=i-1
		}
		v(i+1) = key

		log.debug("final:")
		log.debug(v.toString())
		log.debug("=================================" format j)
 	}
 	v
}

def bubbleSort(v: ArrayBuffer[Int]) : ArrayBuffer[Int] = {
	def swap(i: Int, j: Int) = {
		val t = v(i)
		v(i) = v(j)
		v(j) = t
	}

	for(i <- 0 until v.length) {
		for(j <- 1 until v.length-i) {
			log.debug("%d - %d".format(i, j))
			if(v(j-1)>v(j)) {
				swap(j-1, j)
				log.debug("Swaping %d and %d and got: %s".format(j-1, j, v.toString()))
			} else {
				log.debug("Pass swap, got: %s" format v.toString())
			}
		}
	}
	v
}



def quickSort(v: ArrayBuffer[Int]) : ArrayBuffer[Int] = {
	def swap(i: Int, j: Int) = {
		val t = v(i)
		v(i) = v(j)
		v(j) = t
	}

	def quickSortPartition(low: Int, high: Int) = {
		def strsub(from: Int, to:Int) = v.view(from, to+1).map(_.toString + ", ").reduce(_ + _)
		
		log.debug("Making partition from %d to %d".format(low, high))
		log.debug("\t(%s)".format(strsub(low, high)))
		val pivot = v(high)
		log.debug("Choosed pivot at index %d: %d".format(high, pivot))
		var i = low
		log.debug("Low bound: %d".format(i))
		for(j <- low to high-1) {
			if(v(j) <= pivot) {
				log.debug("Found %d less than pivot, swapping %d and %d".format(v(j), i, j))
				if(i != j) {
					swap(i, j)
				}
				log.debug("(%s)".format(strsub(low, high)))
				i = i + 1
				log.debug("Incremented low bound: %d".format(i))
			}
		}
		swap(i, high)
		log.debug("Final partition result: (%s)".format(strsub(low, high)))
		i
	}

	def quickSortRoutine(low: Int, high: Int): ArrayBuffer[Int] = {
		if(low<high) {
			val p = quickSortPartition(low, high)
			quickSortRoutine(low, p-1)
			quickSortRoutine(p+1, high)
		}
		v
	}
	quickSortRoutine(0, v.length-1)
}

def mergeSort(v: ArrayBuffer[Int]) : ArrayBuffer[Int] = {
	def strsub(from: Int, to:Int) = {
    var s=""
    for(idx <- from.to(to)) {
      s += ", %d".format(v(idx))
    }
    s
  }
		
	var dst = v
	def topDownMerge(from: Int, mid: Int, to: Int) = {
		log.debug("Making merge %s and %s".format(strsub(from, mid), strsub(mid, from)))
		var ifrom = from
		var imid = mid
		for(i <- from until to) {
			if(v(ifrom)<=v(imid)) {
				dst(i) = v(ifrom)
				ifrom = ifrom + 1
			} else {
				dst(i) = v(imid)
				imid = imid + 1
			}
		}
	}
	
	def topDownMergeSort(from: Int, to:Int): ArrayBuffer[Int] = {
		if(to-from < 2) {
			return v
		}
		val mid = (to+from)/2
		log.debug("Top down, from %d to %d and middle %d".format(from, to, mid))
		topDownMergeSort(from, mid)
		topDownMergeSort(mid, from)
		topDownMerge(from, mid, to)
		v
	}
	topDownMergeSort(0, v.length)
	dst
}

if(args.length != 1) {
	throw new IllegalArgumentException("Need sort name as first arg")
}

val res = args(0) match {
	case "bubble" => Some(bubbleSort(arr))
	case "insertion" => Some(insertionSort(arr))
	case "quick" => Some(quickSort(arr))
	case "merge" => Some(mergeSort(arr))
	case _ => println("Select on of: insertion, bubble"); None
}

println(res)
