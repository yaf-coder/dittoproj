import { useState } from 'react'
import { useNavigate, Link } from 'react-router-dom'
import { motion } from 'framer-motion'
import { api } from '../api/client'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'
import Input  from '../components/Input'

export default function Login() {
  const { login } = useAuth()
  const navigate  = useNavigate()

  const [form, setForm]   = useState({ phone_number: '', password: '' })
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)

  function handleChange(e) {
    setError('')
    setForm(f => ({ ...f, [e.target.name]: e.target.value }))
  }

  async function handleSubmit(e) {
    e.preventDefault()
    if (!form.phone_number.trim()) { setError('Phone number is required.'); return }
    if (!form.password)            { setError('Password is required.'); return }

    setLoading(true)
    try {
      const { token } = await api.login({
        phone_number: form.phone_number.trim(),
        password:     form.password,
      })
      await login(token)
      navigate('/questions', { replace: true })
    } catch (e) {
      setError(e.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="h-dvh flex flex-col px-6 bg-white">
      <div className="pt-14 pb-8">
        <motion.h1
          initial={{ opacity: 0, y: 16 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5 }}
          className="text-[32px] font-bold tracking-tight2 text-apple-text mb-1"
        >
          Welcome back
        </motion.h1>
        <p className="text-[17px] text-apple-sub">Log in to your account.</p>
      </div>

      <form onSubmit={handleSubmit} className="flex-1 flex flex-col gap-4">
        <Input
          name="phone_number"
          label="Phone number"
          placeholder="Phone number"
          type="text"
          autoComplete="tel"
          value={form.phone_number}
          onChange={handleChange}
        />
        <Input
          name="password"
          label="Password"
          placeholder="Password"
          type="password"
          autoComplete="current-password"
          value={form.password}
          onChange={handleChange}
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

        <div className="mt-auto pb-10 flex flex-col gap-3">
          <Button fullWidth type="submit" disabled={loading}>
            {loading ? 'Logging in…' : 'Log in'}
          </Button>
          <p className="text-center text-[15px] text-apple-sub">
            Don't have an account?{' '}
            <Link to="/register" className="text-apple-blue font-medium">Sign up</Link>
          </p>
        </div>
      </form>
    </div>
  )
}
