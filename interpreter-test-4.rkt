(define tada
    (lambda (x y)
        (cond ((> x (+ 1 y)) #t)
            ((< x (+ 1 y)) #f)
            (else #f))))
            
(tada 4 3)
(tada 6 6.0)

(cond ((tada 6 4) "this worked!")
    ((> 3 8) "shouldn't happen")
    (else "probably shouldn't happen"))