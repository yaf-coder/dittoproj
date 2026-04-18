import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { motion } from 'framer-motion'
import { api } from '../api/client'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import Input  from '../components/Input'

const fadeUp = (delay = 0) => ({
  initial: { opacity: 0, y: 24 },
  animate: { opacity: 1, y: 0 },
  transition: { duration: 0.6, delay, ease: [0.25, 0.46, 0.45, 0.94] },
})

export default function Landing() {
  const { login }   = useAuth()
  const navigate    = useNavigate()
  const [name, setName]         = useState('')
  const [password, setPassword] = useState('')
  const [error, setError]       = useState('')
  const [loading, setLoading]   = useState(false)

  async function handleSubmit(e) {
    e.preventDefault()
    if (!name.trim())     { setError('First name is required.'); return }
    if (!password.trim()) { setError('Password is required.');   return }

    setLoading(true)
    setError('')
    try {
      const { token } = await api.login({ name: name.trim(), password })
      login(token)
      navigate('/onboarding', { replace: true })
    } catch {
      setError('Something went wrong. Please try again.')
    } finally {
      setLoading(false)
    }
  }

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

        <motion.p {...fadeUp(0.2)} className="text-[20px] font-medium text-apple-sub mb-10">
          Find your match.
        </motion.p>

        {/* Login form */}
        <motion.form
          {...fadeUp(0.3)}
          onSubmit={handleSubmit}
          className="w-full max-w-sm flex flex-col gap-3 text-left"
        >
          <Input
            label="First name"
            placeholder="John"
            type="text"
            autoComplete="given-name"
            value={name}
            onChange={e => { setError(''); setName(e.target.value) }}
          />
          <Input
            label="Password"
            placeholder="••••••••"
            type="password"
            autoComplete="current-password"
            value={password}
            onChange={e => { setError(''); setPassword(e.target.value) }}
          />

          {error && (
            <motion.p
              initial={{ opacity: 0, y: -6 }}
              animate={{ opacity: 1, y: 0 }}
              className="text-apple-red text-[13px] px-1"
            >
              {error}
            </motion.p>
          )}

          <Button fullWidth type="submit" disabled={loading} className="mt-1">
            {loading ? 'Signing in…' : 'Continue'}
          </Button>
        </motion.form>
      </div>

      <motion.p
        {...fadeUp(0.4)}
        className="text-center text-[13px] text-apple-sub/50 px-8 pb-10"
      >
        By continuing you agree to our Terms of Service and Privacy Policy.
      </motion.p>
    </div>
  )
}
