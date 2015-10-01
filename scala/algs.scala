#!/usr/bin/env scalas

/***
scalaVersion := "2.11.7"

libraryDependencies +=  "com.typesafe.scala-logging" %% "scala-logging" % "3.1.0"
libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.1.2"
*/

import scala.util.control.Breaks._
import org.slf4j.LoggerFactory
import com.typesafe.scalalogging.Logger
import scala.math.BigInt

val log = Logger(LoggerFactory.getLogger("MainLog"))

def powrec(base: Double, pow: Int, acc: Double): Double = pow match {
	case 0 => 1
	case 1 => acc
	case _ => powrec(base, pow-1, base*acc)
}

def eueler1() {
	var i3 = 0
	var i5 = 0
	var acc = 0
	val tops = 1000

	breakable {
		while(true) {
			var v3 = i3*3
			var v5 = i5*5
			if(v3<tops) {
				acc += v3
			}
			if(v5<tops) {
				acc += v5
			}
			i3+=1
			i5+=1

			if( (v5>=1000) && (v3 >= 1000)) {
				break
			}
		}
	}
}

def fibo_seq(top: Int): BigInt = {
	var prev = 0
	var before_prev = 1

	var fibo = 0

	var even_acc: BigInt = 0

	var iter = 0
	while(fibo < top) {
		fibo = prev + before_prev
		before_prev = prev

		// log.debug("%d".format(fibo))

		prev = fibo
		if(fibo % 2 == 0) {
			even_acc += fibo
			// log.debug("%d".format(even_acc))
		}
		iter += 1
	}
	return even_acc
}

println(fibo_seq(4000000))
// println(acc.toString())

def fiborec(v: Int): Int = v match {
	case 0 => 0
	case 1 => 1
	case _ => fiborec(v-1) + fiborec(v-2)
}
println(fiborec(10))

