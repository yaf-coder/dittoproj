import { useEffect } from 'react'
import { useNavigate, useSearchParams } from 'react-router-dom'
import { useAuth } from '../store/AuthContext'

export default function AuthCallback() {
  const [params]   = useSearchParams()
  const { login }  = useAuth()
  const navigate   = useNavigate()

  useEffect(() => {
    const token = params.get('token')
    if (!token) { navigate('/', { replace: true }); return }

    login(token)
    // Give AuthContext time to load the user, then route appropriately.
    // Onboarding checks whether profile fields are filled.
    setTimeout(() => navigate('/onboarding', { replace: true }), 300)
  }, [])

  return (
    <div className="h-dvh flex items-center justify-center">
      <div className="flex flex-col items-center gap-4">
        <div className="w-10 h-10 border-[3px] border-apple-blue border-t-transparent rounded-full animate-spin" />
        <p className="text-apple-sub text-[15px]">Signing you in…</p>
      </div>
    </div>
  )
}
