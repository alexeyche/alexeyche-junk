
import com.typesafe.scalalogging.Logger
import com.typesafe.scalalogging.LazyLogging
import org.slf4j.LoggerFactory

import scala.util.Random
import scala.reflect.ClassTag
import scala.collection.mutable.ArrayBuffer


trait SortAlgo[T] extends LazyLogging {
	def sort(a: Array[T]): Array[T] = {
		var v = a.clone()
		sortImpl(v)
		return v
	}
	def sortImpl(a: Array[T])
}

class BadSort[T] extends SortAlgo[T] {
	def sortImpl(a: Array[T]) = ()
}

class InsertionSort[T <% Ordered[T]] extends SortAlgo[T] {
	def sortImpl(a: Array[T]) = {
		for(j <- 1 until a.length) {
			logger.debug("%d iteration =================================" format j)
			val key = a(j)
			var i = j-1
			while( (i>=0) && (a(i)>key)) {
				a(i+1) = a(i)
				logger.debug("i: %d" format i)
				logger.debug(a mkString ", ")
				
				i=i-1
			}
			a(i+1) = key

			logger.debug("final:")
			logger.debug(a mkString ", ")
			logger.debug("=================================" format j)
		}

	}
}


object Sorts {
	private val N = 10

	val log = Logger(LoggerFactory.getLogger(Sorts.getClass.getName))

 	def main(args: Array[String]) {
  		println("Hello, I am mega sort program!")
		if(args.length == 0) {
			println("I am wating for call one of my classes in first arg")
			return
		}
		val algoName = args(0)

		Random.setSeed(1)
		
		val arr = Seq.fill(N)(Random.nextGaussian).toArray
		log.debug("Got sequence: %s" format(arr.mkString(", ")))
		
  		val algo = algoName match {
  			case "BadSort" => Some(new BadSort[Double])
  			case "InsertionSort" => Some(new InsertionSort[Double])
  			case _ => None
  		}
  		
  		if(algo isEmpty) {
  			println("I am sorry, I am don't understand what that %s means" format algoName)
  			return
  		}

  		log.debug("Sorting! Yay")
  		val new_arr = algo.get.sort(arr)
  		log.debug("Done")
  		log.debug("%s".format(new_arr.mkString(", ")))
  	}
}


// class InsertionSort extends SortAlgo {

// }
