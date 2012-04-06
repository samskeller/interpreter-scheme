(define mult
  (lambda (x y z)
    (* x y z)))

(define curry3 ; Function that currys a three-argument function.
  (lambda (f)
    (lambda (x)
      (lambda (y)
        (lambda (z)
          (f x y z))))))
          
(curry3 mult)
((((curry3 mult) 1) 2) 3)