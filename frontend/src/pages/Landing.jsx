import { motion } from 'framer-motion'
import { useNavigate } from 'react-router-dom'
import Button from '../components/Button'

const fadeUp = (delay = 0) => ({
  initial: { opacity: 0, y: 24 },
  animate: { opacity: 1, y: 0 },
  transition: { duration: 0.6, delay, ease: [0.25, 0.46, 0.45, 0.94] },
})

export default function Landing() {
  const navigate = useNavigate()

  return (
    <div className="h-dvh flex flex-col bg-white">
      {/* Hero */}
      <div className="flex-1 flex flex-col items-center justify-center px-8 text-center">
        <motion.div {...fadeUp(0)} className="mb-8">
          <div className="w-20 h-20 rounded-[22px] bg-apple-blue flex items-center justify-center shadow-apple">
            <svg width="40" height="40" viewBox="0 0 40 40" fill="none">
              <path
                d="M20 8C14 8 9 13 9 19c0 4 2.5 7.5 6 9.5L20 32l5-3.5c3.5-2 6-5.5 6-9.5 0-6-5-11-11-11z"
                fill="white"
              />
              <circle cx="16" cy="19" r="2.5" fill="#007AFF" />
              <circle cx="24" cy="19" r="2.5" fill="#007AFF" />
            </svg>
          </div>
        </motion.div>

        <motion.h1
          {...fadeUp(0.1)}
          className="text-[52px] font-bold tracking-tight2 text-apple-text leading-none mb-3"
        >
          Ditto
        </motion.h1>

        <motion.p {...fadeUp(0.2)} className="text-[22px] font-medium text-apple-sub mb-4">
          Find your match.
        </motion.p>

        <motion.p {...fadeUp(0.3)} className="text-[17px] text-apple-sub/80 max-w-xs leading-relaxed">
          Connect with people who truly align with who you are.
        </motion.p>
      </div>

      {/* CTA */}
      <motion.div
        initial={{ opacity: 0, y: 32 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.6, delay: 0.4, ease: [0.25, 0.46, 0.45, 0.94] }}
        className="px-6 pb-12 flex flex-col gap-3"
      >
        <Button fullWidth className="py-4 rounded-2xl text-[17px]" onClick={() => navigate('/register')}>
          Create account
        </Button>
        <Button fullWidth variant="secondary" className="py-4 rounded-2xl text-[17px]" onClick={() => navigate('/login')}>
          Log in
        </Button>

        <p className="text-center text-[13px] text-apple-sub/60 px-4 mt-1">
          By continuing you agree to our Terms of Service and Privacy Policy.
        </p>
      </motion.div>
    </div>
  )
}
