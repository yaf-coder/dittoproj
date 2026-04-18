import { motion } from 'framer-motion'
import Button from '../components/Button'

const fadeUp = (delay = 0) => ({
  initial: { opacity: 0, y: 24 },
  animate: { opacity: 1, y: 0 },
  transition: { duration: 0.6, delay, ease: [0.25, 0.46, 0.45, 0.94] },
})

export default function Landing() {
  return (
    <div className="h-dvh flex flex-col bg-white">
      {/* Hero */}
      <div className="flex-1 flex flex-col items-center justify-center px-8 text-center">
        {/* Logo mark */}
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
        <a href="/auth/google" className="block">
          <Button fullWidth className="py-4 rounded-2xl text-[17px]">
            {/* Google logo */}
            <svg width="20" height="20" viewBox="0 0 24 24">
              <path fill="#fff" d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z"/>
              <path fill="#fff" d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z"/>
              <path fill="#fff" d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l3.66-2.84z"/>
              <path fill="#fff" d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z"/>
            </svg>
            Continue with Google
          </Button>
        </a>

        <p className="text-center text-[13px] text-apple-sub/60 px-4">
          By continuing you agree to our Terms of Service and Privacy Policy.
        </p>
      </motion.div>
    </div>
  )
}
