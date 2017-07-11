
def gauss(x, xc, sigma):
    return (1.0/(np.sqrt(2.0*np.pi)*sigma)) * np.exp( - ( (x-xc)*(x-xc) )/(2.0*sigma*sigma) )

def build_dog(pre_size, post_size, amp_pos=1.0, amp_neg=1.0, sigma_pos=0.5, sigma_neg=2):
    w = np.zeros((pre_size, post_size))
    for pre_id in xrange(pre_size):
        for post_id in xrange(post_size):
            post_circled = post_id-post_size
            pre_circled = pre_id-pre_size

            w[pre_id, post_id] += amp_pos * gauss(pre_id, post_id, sigma_pos) - amp_neg * gauss(pre_id, post_id, sigma_neg)
            w[pre_id, post_id] += amp_pos * gauss(pre_id, post_circled, sigma_pos) - amp_neg * gauss(pre_id, post_circled, sigma_neg)
            w[pre_id, post_id] += amp_pos * gauss(pre_circled, post_id, sigma_pos) - amp_neg * gauss(pre_circled, post_id, sigma_neg)
    return w
