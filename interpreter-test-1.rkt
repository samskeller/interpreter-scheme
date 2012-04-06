(define factorial 
    (lambda (a)
        (if (= a 1)
            1
            (* a (factorial (- a 1))))))

(factorial 4)