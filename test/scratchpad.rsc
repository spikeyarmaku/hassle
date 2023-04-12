(let x 1 (let f (lambda y x) (let x 2 (f 0))))

(
    (let make-closure (lambda value (print value)))
    (let doughnut (make-closure "doughnut"))
    (let bagel (make-closure "bagel"))
    
    (sequence doughnut bagel)
)